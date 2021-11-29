#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/uthreads.h"

void foo() {
    int c = 0;
    for (int i=0;i<5;i++) {
        printf("foo (tid=%d): %d\n", mytid(), c);
        c += 1;
        yield();
        if (i==2) return;
    }
}

void bar() {
    int c = 0;
    for (int i=0;i<5;i++) {
        printf("bar (tid=%d): %d\n", mytid(), c);
        yield();
        c += 2;
    }
}

int main() {
    make_uthread(foo);
    make_uthread(bar);
    start_uthreads();
    printf("next line of 1st start_uthreads\n");
    start_uthreads();
    printf("next line of 2nd start_uthreads\n");
    exit(0);
}
