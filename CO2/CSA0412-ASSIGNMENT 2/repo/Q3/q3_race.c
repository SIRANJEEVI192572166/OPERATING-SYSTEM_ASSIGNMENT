#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sched.h>
#define THREADS 4
#define PER_THREAD 2000000L

static volatile long unsafe_counter = 0;       /* no protection                     */
static long mutex_counter = 0;                 /* protected by a mutex              */
static atomic_long atomic_counter = 0;         /* C11 atomic                        */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int inject_yield = 0;     /* 1: force a context switch between load and store */

static void *worker(void *arg)
{
    (void)arg;
    for (long i = 0; i < PER_THREAD; i++) {
        long tmp = unsafe_counter;             /* load                              */
        if (inject_yield && (i & 1023) == 0) sched_yield();    /* let another thread run mid-update */
        unsafe_counter = tmp + 1;              /* store: overwrites others' updates */
        pthread_mutex_lock(&lock); mutex_counter++; pthread_mutex_unlock(&lock);
        atomic_fetch_add(&atomic_counter, 1);
    }
    return NULL;
}
int main(int argc, char **argv)
{
    (void)argv; inject_yield = argc > 1;
    printf("Mode: %s\n", inject_yield ? "forced interleaving (sched_yield between load and store)" : "natural scheduling");
    pthread_t th[THREADS];
    for (int i = 0; i < THREADS; i++) pthread_create(&th[i], NULL, worker, NULL);
    for (int i = 0; i < THREADS; i++) pthread_join(th[i], NULL);
    long expected = THREADS * PER_THREAD;
    printf("Expected value        : %ld\n", expected);
    printf("Unprotected counter   : %ld   (lost updates: %ld)\n", unsafe_counter, expected - unsafe_counter);
    printf("Mutex-protected       : %ld   (lost updates: %ld)\n", mutex_counter, expected - mutex_counter);
    printf("Atomic (fetch_add)    : %ld   (lost updates: %ld)\n", (long)atomic_counter, expected - (long)atomic_counter);
    return 0;
}
