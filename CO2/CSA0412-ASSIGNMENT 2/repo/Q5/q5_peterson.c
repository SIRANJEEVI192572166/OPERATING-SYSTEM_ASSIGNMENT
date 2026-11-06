/* Peterson's solution for two threads. Default: C11 seq_cst atomics (correct). -DPLAIN: plain volatile variables (no fences) */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#define N 20000
#ifdef PLAIN
static volatile int flag[2], turn;
#define FLAG_SET(i,v) (flag[i] = (v))
#define FLAG_GET(i)   (flag[i])
#define TURN_SET(v)   (turn = (v))
#define TURN_GET()    (turn)
#else
static atomic_int flag[2], turn;
#define FLAG_SET(i,v) atomic_store(&flag[i], (v))
#define FLAG_GET(i)   atomic_load(&flag[i])
#define TURN_SET(v)   atomic_store(&turn, (v))
#define TURN_GET()    atomic_load(&turn)
#endif
static long counter = 0;
static void *proc(void *arg)
{
    int i = (int)(long)arg, j = 1 - i;
    for (int k = 0; k < N; k++) {
        FLAG_SET(i, 1); TURN_SET(j);                 /* entry section */
        while (FLAG_GET(j) && TURN_GET() == j) ;      /* busy wait     */
        counter++;                                    /* critical section */
        FLAG_SET(i, 0);                               /* exit section  */
    }
    return NULL;
}
int main(void)
{
    pthread_t t0, t1;
    pthread_create(&t0, NULL, proc, (void *)0L); pthread_create(&t1, NULL, proc, (void *)1L);
    pthread_join(t0, NULL); pthread_join(t1, NULL);
#ifdef PLAIN
    printf("Peterson (plain volatile, no fences): counter = %ld, expected %d\n", counter, 2 * N);
#else
    printf("Peterson (C11 seq_cst atomics): counter = %ld, expected %d\n", counter, 2 * N);
#endif
    return 0;
}
