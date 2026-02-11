#include <stdio.h>

#include "buddy.h"

int main(void) {
    printf("Buddy allocator tests...\n");
    buddy_test();

    printf("All tests successful!\n");
    return 0;
}
