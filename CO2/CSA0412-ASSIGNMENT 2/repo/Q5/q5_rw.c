/* Readers-writers (readers' preference) and dining philosophers, both with semaphores */
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <unistd.h>

/* ---------- readers-writers ---------- */
static sem_t rw_mutex, mutex_rc; static int read_count = 0, A = 0, B = 0, bad_reads = 0, reads_done = 0;
static void *writer(void *a){ (void)a; for (int i = 0; i < 2000; i++) { sem_wait(&rw_mutex); A++; sched_yield(); B++; sem_post(&rw_mutex); } return NULL; }
static void *reader(void *a)
{
    (void)a;
    for (int i = 0; i < 2000; i++) {
        sem_wait(&mutex_rc); if (++read_count == 1) sem_wait(&rw_mutex); sem_post(&mutex_rc);
        if (A != B) bad_reads++;                           /* a reader must never see A != B */
        sched_yield();
        sem_wait(&mutex_rc); reads_done++; if (--read_count == 0) sem_post(&rw_mutex); sem_post(&mutex_rc);
    }
    return NULL;
}
/* ---------- dining philosophers: room semaphore lets at most 4 sit, so a cycle cannot form ---------- */
#define PHIL 5
static sem_t fork_s[PHIL], room; static int meals[PHIL];
static void *philosopher(void *a)
{
    int i = (int)(long)a;
    for (int m = 0; m < 3; m++) {
        sem_wait(&room);
        sem_wait(&fork_s[i]); sem_wait(&fork_s[(i + 1) % PHIL]);
        meals[i]++; usleep(1000);                          /* eat */
        sem_post(&fork_s[(i + 1) % PHIL]); sem_post(&fork_s[i]);
        sem_post(&room);
        usleep(500);                                       /* think */
    }
    return NULL;
}
int main(void)
{
    sem_init(&rw_mutex, 0, 1); sem_init(&mutex_rc, 0, 1);
    pthread_t r[3], w[2];
    for (int i = 0; i < 2; i++) pthread_create(&w[i], NULL, writer, NULL);
    for (int i = 0; i < 3; i++) pthread_create(&r[i], NULL, reader, NULL);
    for (int i = 0; i < 2; i++) pthread_join(w[i], NULL);
    for (int i = 0; i < 3; i++) pthread_join(r[i], NULL);
    printf("Readers-writers: 3 readers x 2000 reads, 2 writers x 2000 writes -> reads done = %d, inconsistent reads = %d, final A = %d, B = %d\n", reads_done, bad_reads, A, B);

    for (int i = 0; i < PHIL; i++) sem_init(&fork_s[i], 0, 1);
    sem_init(&room, 0, PHIL - 1);
    pthread_t ph[PHIL];
    for (long i = 0; i < PHIL; i++) pthread_create(&ph[i], NULL, philosopher, (void *)i);
    for (int i = 0; i < PHIL; i++) pthread_join(ph[i], NULL);
    printf("Dining philosophers: meals eaten =");
    for (int i = 0; i < PHIL; i++) printf(" %d", meals[i]);
    printf("  (each philosopher finished, no deadlock)\n");
    return 0;
}
