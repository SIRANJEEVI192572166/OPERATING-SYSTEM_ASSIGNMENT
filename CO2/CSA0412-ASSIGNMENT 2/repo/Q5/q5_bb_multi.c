/* Bounded buffer, N producers and N consumers; verifies every item is consumed exactly once.
   Build: -DNO_MUTEX removes the mutex; -DINJECT adds a forced context switch inside the unprotected region. */
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <sched.h>
#define SIZE 5
#define ITEMS 5000                         /* items per producer */

static int buffer[SIZE], in = 0, out = 0, np, nc;
static sem_t empty, full;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static atomic_int *seen;                   /* seen[id] = how many times item id was consumed */
#ifdef NO_MUTEX
#define LOCK()
#define UNLOCK()
#else
#define LOCK() pthread_mutex_lock(&lock)
#define UNLOCK() pthread_mutex_unlock(&lock)
#endif
#ifdef INJECT
#define MAYBE_YIELD() sched_yield()
#else
#define MAYBE_YIELD()
#endif

static void *producer(void *arg)
{
    long id = (long)arg;
    for (int k = 0; k < ITEMS; k++) {
        sem_wait(&empty); LOCK();
        int idx = in; MAYBE_YIELD();
        buffer[idx] = (int)(id * ITEMS + k); in = (idx + 1) % SIZE;
        UNLOCK(); sem_post(&full);
    }
    return NULL;
}
static void *consumer(void *arg)
{
    (void)arg;
    for (int k = 0; k < np * ITEMS / nc; k++) {
        sem_wait(&full); LOCK();
        int idx = out; MAYBE_YIELD();
        int item = buffer[idx]; out = (idx + 1) % SIZE;
        UNLOCK(); sem_post(&empty);
        atomic_fetch_add(&seen[item], 1);
    }
    return NULL;
}
int main(int argc, char **argv)
{
    np = nc = argc > 1 ? atoi(argv[1]) : 1;
    int total = np * ITEMS; seen = calloc((size_t)total, sizeof *seen);
    sem_init(&empty, 0, SIZE); sem_init(&full, 0, 0);
    pthread_t p[16], c[16];
    for (long i = 0; i < np; i++) pthread_create(&p[i], NULL, producer, (void *)i);
    for (long i = 0; i < nc; i++) pthread_create(&c[i], NULL, consumer, NULL);
    for (int i = 0; i < np; i++) pthread_join(p[i], NULL);
    for (int i = 0; i < nc; i++) pthread_join(c[i], NULL);
    int once = 0, lost = 0, dup = 0;
    for (int i = 0; i < total; i++) { int s = seen[i]; if (s == 1) once++; else if (s == 0) lost++; else dup++; }
    printf("%d producer(s), %d consumer(s): items produced = %d, consumed exactly once = %d, never consumed = %d, consumed more than once = %d  -> %s\n",
           np, nc, total, once, lost, dup, (once == total) ? "CORRECT" : "WRONG");
    return 0;
}
