//
// Created by Leonard on 2026-02-10.
//

#include "buddy.h"

#include <sys/mman.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>


#define MIN_LEVEL 3
#define PAGE_LEVEL 12 // assume 4 KiB = 4096 bytes = 2^12 bytes

#define PAGE_LEN (1 << PAGE_LEVEL)
#define ONES 0xffffffffffffffff

typedef struct head {
    bool taken;
    short level;
} head_t;

static_assert(
    1 << MIN_LEVEL >= sizeof(head_t),
    "Min block size should be at least size of block head");

head_t *new_block() {
    head_t *block = mmap(
        NULL, PAGE_LEN, PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANON, -1, 0);
    if (block == MAP_FAILED) {
        return NULL;
    }

    // page-aligned
    assert((((size_t) block) & ~(ONES << PAGE_LEVEL)) == 0x0);
    block->level = PAGE_LEVEL;
    return block;
}

head_t *get_next(head_t const *block) {
    return (head_t *) ((size_t) block + (0x1 << block->level));
}

head_t *get_buddy(head_t const *block) {
    return (head_t *) ((size_t) block ^ (0x1 << block->level));
}

head_t *split(head_t *block) {
    short new_level = block->level - 1;
    *block = (head_t) { .taken = false, .level = new_level};
    head_t *other = (head_t *) ((size_t) block | (0x1 << new_level));
    *other = *block;
    return other;
}

head_t *merge(head_t *block) {
    long mask = ONES << (block->level + 1);
    head_t *primary = (head_t *) ((size_t) block & mask);
    primary->level++;
    return primary;
}

short level(size_t alloc_len) {
    size_t total_len = alloc_len + sizeof(head_t);
    // equivalent to ceil(log2(total_len))
    // total_len - 1 > 0 is guaranteed
    int level = 8 * sizeof(long) - __builtin_clzl((long) total_len - 1);
    // max(MIN_LEVEL, bit_width - 1); bit_width == ceil(log2(total_len))
    if (level > PAGE_LEVEL) {
        return -1;
    }
    return level < MIN_LEVEL ? MIN_LEVEL : level;
}

void buddy_test() {
    head_t *block = new_block();
    assert(block != NULL); // required for rest of tests to pass
    assert(block->level == PAGE_LEVEL);

    head_t *buddy_1 = split(block);
    assert(block->level == PAGE_LEVEL - 1);
    assert(buddy_1->level == PAGE_LEVEL - 1);
    assert((size_t) buddy_1 == (size_t) block + PAGE_LEN / 2);
    assert(get_buddy(block) == buddy_1);
    assert(get_buddy(buddy_1) == block);
    assert(get_next(block) == buddy_1);

    head_t *buddy_2 = split(block);
    assert(block->level == PAGE_LEVEL - 2);
    assert(buddy_1->level == PAGE_LEVEL - 1);
    assert(buddy_2->level == PAGE_LEVEL - 2);
    assert((size_t) buddy_2 == (size_t) block + PAGE_LEN / 4);
    assert(get_buddy(block) == buddy_2);
    assert(get_buddy(buddy_1) == block);
    assert(get_next(block) == buddy_2);
    assert(get_next(buddy_2) == buddy_1);

    head_t *block_1 = merge(buddy_2);
    assert(block->level == PAGE_LEVEL - 1);
    assert(buddy_1->level == PAGE_LEVEL - 1);
    assert(block == block_1);

    head_t *block_2 = merge(buddy_1);
    assert(block->level == PAGE_LEVEL);
    assert(block == block_2);

    assert(level(0) == MIN_LEVEL);
    assert(level(sizeof(head_t)) == MIN_LEVEL);
    assert(level(200 - sizeof(head_t)) == 8);
    assert(level(256 - sizeof(head_t)) == 8);
    assert(level(257 - sizeof(head_t)) == 9);
}
