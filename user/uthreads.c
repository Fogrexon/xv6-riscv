#include "kernel/types.h"
#include "user/user.h"
#include "user/uthreads.h"

// copied from kernel/defs.h
void swtch(struct context*, struct context*);

#define MAX_UTHREADS 8
#define STACK_DEPTH 512

// thread table
struct uthread uthread[MAX_UTHREADS];

// thread stacks
__attribute__ ((aligned (16)))
uint64 stack[MAX_UTHREADS][STACK_DEPTH];

// current thread id
int tid;

int
make_uthread(void (*fun)())
{
  return -1;  // DUMMY (SHOULD BE REMOVED)
  /*
   * FILL YOUR CODE HERE
   */
}

void
start_uthreads()
{
  /*
   * FILL YOUR CODE HERE
   */
}

void
yield()
{
  /*
   * FILL YOUR CODE HERE
   */
}

int
mytid() {
  return tid;
}
