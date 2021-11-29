#include "kernel/types.h"
#include "user/user.h"
#include "user/uthreads.h"

// copied from kernel/defs.h
void swtch(struct context*, struct context*);

#define MAX_UTHREADS 8
#define STACK_DEPTH 512

// thread table
struct uthread uthread[MAX_UTHREADS];

// main context
struct context main_context;

// thread stacks
__attribute__ ((aligned (16)))
uint64 stack[MAX_UTHREADS][STACK_DEPTH];

// current thread id
int tid;

int
make_uthread(void (*fun)())
{
  int use_tid = -1;
  for (int i=0;i<MAX_UTHREADS;i++) {
    if (uthread[i].state == UT_UNUSED) {
      use_tid = i;
      break;
    }
  }
  if (use_tid < 0) return -1;
  
  uthread[use_tid].context.ra = (uint64)fun;
  uint64 *sp = &(stack[use_tid] + STACK_DEPTH);
  *--sp = (uint64)uthread_exit;
  // *(stack[use_tid] + STACK_DEPTH) = (uint64)uthread_exit;
  uthread[use_tid].context.sp = (uint64)sp;
  uthread[use_tid].state = UT_READY;

  return use_tid;  // DUMMY (SHOULD BE REMOVED)
}

void
start_uthreads()
{
  tid = 0;

  swtch(&main_context, &uthread[tid].context);
}

void
func_wrapper()
{
  
}

void
uthread_exit()
{
  printf("exit\n");
  int old_tid = tid;
  uthread[old_tid].state = UT_UNUSED;

  for (int i=tid+1;i<tid+MAX_UTHREADS;i++) {
    if (uthread[i%8].state == UT_READY) {
      tid = i%8;
      uthread[tid].state = UT_RUNNING;
      swtch(&uthread[old_tid].context, &uthread[tid].context);
      return;
    }
  }
  tid = -1;
  uthread[old_tid].state = UT_UNUSED;
  swtch(&uthread[old_tid].context, &main_context);
}

void
yield()
{
  int old_tid = tid;
  uthread[old_tid].state = UT_READY;

  for (int i=tid+1;i<tid+MAX_UTHREADS+1;i++) {
    if (uthread[i%8].state == UT_READY) {
      tid = i%8;
      uthread[tid].state = UT_RUNNING;
      swtch(&uthread[old_tid].context, &uthread[tid].context);
      return;
    }
  }
}

int
mytid() {
  return tid;
}
