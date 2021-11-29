#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/uthreads.h"

void foo() {
  int c = 0;
  for(;;) {
    c += 1;
    for(int i=0;i<3;i++) {
      printf("foo: %d\n", c * i);
      yield();
    }
    yield();
  }
}

void bar() {
  int c = 0;
  for(;;) {
    c += 1;
    printf("bar: %d\n", c);
    yield();
  }
}

int main() {
  make_uthread(foo);
  make_uthread(bar);
  start_uthreads();
  exit(0);
}
