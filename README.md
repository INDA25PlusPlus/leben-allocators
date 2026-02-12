# leben-allocators

Implementations of two standard memory allocator types.

## Buddy allocator

Interface is declared in `buddy.h`. Use `buddy_alloc` and `buddy_free` for memory allocation and deallocation, respectively.

Allows allocation of differently sized regions, and finds regions similar in size to the requested size. `buddy_alloc` will return `NULL` if no free region can be found.

Implementation in `buddy.c`. Loosely based off of [https://people.kth.se/~johanmon/ose/assignments/buddy.pdf](https://people.kth.se/~johanmon/ose/assignments/buddy.pdf).

## Pool allocator

Interface is declared in `pool.h`. Use `pool_alloc` and `pool_free` for memory allocation and deallocation, respectively.

Works by creating an allocator with a specific block size, which can allocate regions guaranteed to be that specific size or larger. Use `new_pool_alloc` to create an allocator, check for success using `new_pool_alloc_failed`, and delete the allocator using `delete_pool_alloc`, freeing all owned memory. `pool_alloc` will return `NULL` if no free block can be found.

Implementation in `pool.c`.

## Usage

Setup CMake: `cmake -B build/`

Run tests: `cmake --build build/ && build/leben-allocators`

