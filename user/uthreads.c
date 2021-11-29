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

// function wrapper
void (*running_fun)();
void
func_wrapper()
{
  running_fun();
  uthread_exit();
}

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
  
  uthread[use_tid].context.ra = (uint64)func_wrapper;
  uthread[use_tid].fun = fun;
  uthread[use_tid].context.sp = (uint64)(stack[use_tid] + STACK_DEPTH);
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
uthread_exit()
{
  printf("exit\n");
  int old_tid = tid;
  uthread[old_tid].state = UT_UNUSED;

  for (int i=tid+1;i<tid+MAX_UTHREADS;i++) {
    if (uthread[i%8].state == UT_READY) {
      tid = i%8;
      uthread[tid].state = UT_RUNNING;
      running_fun = uthread[tid].fun;
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
      running_fun = uthread[tid].fun;
      swtch(&uthread[old_tid].context, &uthread[tid].context);
      return;
    }
  }
}

int
mytid() {
  return tid;
}
