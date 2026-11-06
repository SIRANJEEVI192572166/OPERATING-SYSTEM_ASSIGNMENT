/* Alternative: protect a shared counter with a mutex, a spin lock, or an atomic fetch_add */
#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>
#include <unistd.h>
#define TOTAL 1600000L                                /* total updates, divided among the threads */
static long plain_counter; static atomic_long atomic_counter; static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static pthread_spinlock_t sp; static int method, nthreads;

static double now(double *cpu){ struct timespec t, c; clock_gettime(CLOCK_MONOTONIC,&t); clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&c); *cpu = c.tv_sec + c.tv_nsec*1e-9; return t.tv_sec+t.tv_nsec*1e-9; }
static void *worker(void *a)
{
    (void)a;
    for (long i = 0; i < TOTAL / nthreads; i++) {
        if (method == 0) { pthread_mutex_lock(&m); plain_counter++; pthread_mutex_unlock(&m); }
        else if (method == 1) { pthread_spin_lock(&sp); plain_counter++; pthread_spin_unlock(&sp); }
        else atomic_fetch_add(&atomic_counter, 1);
    }
    return NULL;
}
int main(void)
{
    pthread_spin_init(&sp, PTHREAD_PROCESS_PRIVATE);
    const char *nm[3] = {"Mutex", "Spin lock", "Atomic fetch_add"}; int ths[3] = {1, 4, 16};
    printf("%d updates in total, CPUs online: %ld\n%-18s %8s %12s %14s %12s\n", (int)TOTAL, sysconf(_SC_NPROCESSORS_ONLN), "Method", "Threads", "ns/update", "Wall (ms)", "CPU (ms)");
    for (int k = 0; k < 3; k++) for (int j = 0; j < 3; j++) {
        method = k; nthreads = ths[j]; plain_counter = 0; atomic_store(&atomic_counter, 0);
        pthread_t th[16]; double c0, c1, t0 = now(&c0);
        for (int i = 0; i < nthreads; i++) pthread_create(&th[i], NULL, worker, NULL);
        for (int i = 0; i < nthreads; i++) pthread_join(th[i], NULL);
        double t1 = now(&c1); long v = k == 2 ? (long)atomic_counter : plain_counter;
        printf("%-18s %8d %12.1f %14.1f %12.1f %s\n", nm[k], nthreads, (t1 - t0) * 1e9 / TOTAL, (t1 - t0) * 1e3, (c1 - c0) * 1e3, v == TOTAL ? "" : "WRONG");
    }
    return 0;
}
