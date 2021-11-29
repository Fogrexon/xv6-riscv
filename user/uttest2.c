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
        if (i==2) uthread_exit();
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

void baz_sub(int *cp) {
  printf("baz (tid=%d): %d\n", mytid(), *cp);
  yield();
  *cp += 3;
}

void baz() {
    int c = 0;
    for (int i=0;i<5;i++) {
        baz_sub(&c);
        baz_sub(&c);
        if (i == 4) return;
    }
    printf("this line may not exec.");
}

int main() {
    make_uthread(foo);
    make_uthread(bar);
    make_uthread(baz);
    start_uthreads();
    printf("next line of start_uthreads\n");
    exit(0);
}
