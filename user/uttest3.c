#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/uthreads.h"

int THREAD_WAIT_KEY  = 1;
int THREAD_WAIT_KEY2 = 2;

void foo() {
    int c = 0;
    printf("foo (tid=%d): c=%d\n", mytid(), c);
    yield();
    c++;
    printf("foo (tid=%d): c=%d\n", mytid(), c);
    yield();
    c++;
    printf("foo (tid=%d): c=%d\n", mytid(), c);
    yield();

    printf("foo (tid=%d): wake bar and baz up\n", mytid());
    uthread_notify_all(&THREAD_WAIT_KEY);
    c++;
    printf("foo (tid=%d): c=%d\n", mytid(), c);
    yield();

    c++;
    printf("foo (tid=%d): c=%d\n", mytid(), c);
    yield();

    printf("foo (tid=%d): wake quz up\n", mytid());
    uthread_notify(3, &THREAD_WAIT_KEY2);
    c++;
    printf("foo (tid=%d): c=%d\n", mytid(), c);
    yield();

    printf("foo (tid=%d): exit\n", mytid());
}

void bar() {
    int c = 0;
    printf("bar (tid=%d): c=%d\n", mytid(), c);
    yield();

    c++;
    printf("bar (tid=%d): c=%d\n", mytid(), c);

    // wait thread
    printf("bar (tid=%d): waiting...\n", mytid());
    uthread_wait(&THREAD_WAIT_KEY);
    printf("bar (tid=%d): wake up!\n", mytid());

    c++;
    printf("bar (tid=%d): c=%d\n", mytid(), c);
    yield();

    printf("bar (tid=%d): exit\n", mytid());
}

void baz() {
    int c = 0;
    printf("baz (tid=%d): c=%d\n", mytid(), c);
    yield();

    c++;
    printf("baz (tid=%d): c=%d\n", mytid(), c);

    // wait thread
    printf("baz (tid=%d): waiting...\n", mytid());
    uthread_wait(&THREAD_WAIT_KEY);
    printf("baz (tid=%d): wake up!\n", mytid());

    c++;
    printf("baz (tid=%d): c=%d\n", mytid(), c);
    yield();

    printf("baz (tid=%d): exit\n", mytid());
}

void quz() {
    int c = 0;
    printf("quz (tid=%d): c=%d\n", mytid(), c);
    yield();

    c++;
    printf("quz (tid=%d): c=%d\n", mytid(), c);

    // wait thread with key2
    printf("quz (tid=%d): waiting...\n", mytid());
    uthread_wait(&THREAD_WAIT_KEY2);
    printf("quz (tid=%d): wake up!\n", mytid());

    c++;
    printf("quz (tid=%d): c=%d\n", mytid(), c);
    yield();

    printf("quz (tid=%d): exit\n", mytid());
}

int main() {
    make_uthread(foo);
    make_uthread(bar);
    make_uthread(baz);
    make_uthread(quz);
    start_uthreads();
    exit(0);
}
