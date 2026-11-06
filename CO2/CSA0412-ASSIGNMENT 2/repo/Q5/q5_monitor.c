/* Bounded buffer written as a monitor: one mutex + two condition variables */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#define SIZE 5
#define ITEMS 5000
static struct { int buf[SIZE], in, out, count; pthread_mutex_t m; pthread_cond_t not_full, not_empty; } mon =
    { {0}, 0, 0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER };
static atomic_int *seen; static int n;

static void put(int item)                                 /* monitor procedure */
{
    pthread_mutex_lock(&mon.m);
    while (mon.count == SIZE) pthread_cond_wait(&mon.not_full, &mon.m);
    mon.buf[mon.in] = item; mon.in = (mon.in + 1) % SIZE; mon.count++;
    pthread_cond_signal(&mon.not_empty);
    pthread_mutex_unlock(&mon.m);
}
static int get(void)                                      /* monitor procedure */
{
    pthread_mutex_lock(&mon.m);
    while (mon.count == 0) pthread_cond_wait(&mon.not_empty, &mon.m);
    int item = mon.buf[mon.out]; mon.out = (mon.out + 1) % SIZE; mon.count--;
    pthread_cond_signal(&mon.not_full);
    pthread_mutex_unlock(&mon.m);
    return item;
}
static void *producer(void *a){ long id = (long)a; for (int k = 0; k < ITEMS; k++) put((int)(id * ITEMS + k)); return NULL; }
static void *consumer(void *a){ (void)a; for (int k = 0; k < ITEMS; k++) atomic_fetch_add(&seen[get()], 1); return NULL; }
int main(int argc, char **argv)
{
    n = argc > 1 ? atoi(argv[1]) : 1; seen = calloc((size_t)n * ITEMS, sizeof *seen);
    pthread_t p[16], c[16];
    for (long i = 0; i < n; i++) { pthread_create(&p[i], NULL, producer, (void *)i); pthread_create(&c[i], NULL, consumer, NULL); }
    for (int i = 0; i < n; i++) { pthread_join(p[i], NULL); pthread_join(c[i], NULL); }
    int once = 0; for (int i = 0; i < n * ITEMS; i++) if (seen[i] == 1) once++;
    printf("Monitor bounded buffer, %d producer(s)/%d consumer(s): %d of %d items consumed exactly once -> %s\n", n, n, once, n * ITEMS, once == n * ITEMS ? "CORRECT" : "WRONG");
    return 0;
}
