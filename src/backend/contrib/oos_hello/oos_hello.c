#include <stdio.h>

extern void init(void *arg, ...) {
    hello();
}

void hello() {
    printf("Hello world!\n This is my first operation systems lab!\n");
}