/* Seat counter protected by semaphores. Mode 0: no protection, 1: binary semaphore, 2: counting semaphore */
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#define SEATS 100
#define AGENTS 8
#define TRIES 20                         /* each agent tries to book 20 seats: 160 requests for 100 seats */

static int seats_left = SEATS, sold[AGENTS], mode;
static sem_t bin, seats;                 /* bin: binary semaphore (mutex), seats: counting semaphore = free seats */

static void *agent(void *arg)
{
    long id = (long)arg;
    for (int i = 0; i < TRIES; i++) {
        if (mode == 0) {                              /* check-then-act without protection */
            if (seats_left > 0) { int t = seats_left; sched_yield(); seats_left = t - 1; sold[id]++; }
        } else if (mode == 1) {                       /* wait(bin) ... signal(bin) */
            sem_wait(&bin);
            if (seats_left > 0) { int t = seats_left; sched_yield(); seats_left = t - 1; sold[id]++; }
            sem_post(&bin);
        } else {                                      /* each free seat is one token of the counting semaphore */
            if (sem_trywait(&seats) == 0) sold[id]++;
        }
    }
    return NULL;
}
int main(void)
{
    const char *title[3] = {"No protection", "Binary semaphore (wait/signal around check+decrement)", "Counting semaphore initialised to 100"};
    for (mode = 0; mode < 3; mode++) {
        seats_left = SEATS; for (int i = 0; i < AGENTS; i++) sold[i] = 0;
        sem_init(&bin, 0, 1); sem_init(&seats, 0, SEATS);
        pthread_t th[AGENTS];
        for (long i = 0; i < AGENTS; i++) pthread_create(&th[i], NULL, agent, (void *)i);
        for (int i = 0; i < AGENTS; i++) pthread_join(th[i], NULL);
        int total = 0; for (int i = 0; i < AGENTS; i++) total += sold[i];
        printf("%-56s seats sold = %3d of %d  -> %s\n", title[mode], total, SEATS, total == SEATS ? "correct" : "OVERSOLD");
        sem_destroy(&bin); sem_destroy(&seats);
    }
    return 0;
}
