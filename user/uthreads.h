// copied from kernel/proc.h
struct context {
  uint64 ra;
  uint64 sp;

  // callee-saved
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
};

enum ut_state {
  UT_UNUSED,
  UT_READY,
  UT_RUNNING
};

struct uthread { 
  enum ut_state state;
  struct context context;
};

int make_uthread(void (*fun)());
void start_uthreads();
void uthread_exit();
void yield();
int mytid();
