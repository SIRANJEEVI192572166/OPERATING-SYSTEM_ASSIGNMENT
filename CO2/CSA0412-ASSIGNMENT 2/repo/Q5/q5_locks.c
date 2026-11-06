/* Lock granularity for seat booking: one global lock, one lock per train, one lock per coach */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#define TRAINS 8
#define COACHES 10
#define SEATS 72
#define THREADS 8
#define BOOKINGS 1500                           /* booking requests per agent thread */

static unsigned char booked[TRAINS][COACHES][SEATS];
static pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER, train_lock[TRAINS], coach_lock[TRAINS][COACHES];
static int granularity;                          /* 0 global, 1 per train, 2 per coach */
static long io_ns;                               /* time spent inside the critical section writing the booking record */
static long ok_count[THREADS];

static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec*1e-9; }
static pthread_mutex_t *pick(int t, int c){ return granularity == 0 ? &global_lock : granularity == 1 ? &train_lock[t] : &coach_lock[t][c]; }

static void *agent(void *arg)
{
    long id = (long)arg; unsigned s = 12345u + (unsigned)id * 7919u;
    for (int i = 0; i < BOOKINGS; i++) {
        s = s * 1103515245u + 12345u; int t = (s >> 8) % TRAINS;
        s = s * 1103515245u + 12345u; int c = (s >> 8) % COACHES;
        s = s * 1103515245u + 12345u; int seat = (s >> 8) % SEATS;
        pthread_mutex_t *m = pick(t, c);
        pthread_mutex_lock(m);
        if (!booked[t][c][seat]) {
            if (io_ns) nanosleep(&(struct timespec){0, io_ns}, NULL);      /* e.g. journal / database write */
            booked[t][c][seat] = 1; ok_count[id]++;
        }
        pthread_mutex_unlock(m);
    }
    return NULL;
}
static double run(int g, long ns, long *ok, int *taken)
{
    granularity = g; io_ns = ns; memset(booked, 0, sizeof booked); memset(ok_count, 0, sizeof ok_count);
    pthread_t th[THREADS]; double t0 = now();
    for (long i = 0; i < THREADS; i++) pthread_create(&th[i], NULL, agent, (void *)i);
    for (int i = 0; i < THREADS; i++) pthread_join(th[i], NULL);
    double t = now() - t0; *ok = 0; for (int i = 0; i < THREADS; i++) *ok += ok_count[i];
    *taken = 0; for (int a = 0; a < TRAINS; a++) for (int b = 0; b < COACHES; b++) for (int c = 0; c < SEATS; c++) *taken += booked[a][b][c];
    return t;
}
int main(void)
{
    for (int t = 0; t < TRAINS; t++) { pthread_mutex_init(&train_lock[t], NULL); for (int c = 0; c < COACHES; c++) pthread_mutex_init(&coach_lock[t][c], NULL); }
    const char *name[3] = {"One global lock", "One lock per train (8)", "One lock per coach (80)"};
    printf("%d agent threads x %d booking requests, %d trains x %d coaches x %d seats, CPUs online: %ld\n", THREADS, BOOKINGS, TRAINS, COACHES, SEATS, sysconf(_SC_NPROCESSORS_ONLN));
    long ios[2] = {0, 50000};
    for (int k = 0; k < 2; k++) {
        printf("\nCritical section = %s\n%-26s %10s %14s %9s %s\n", k == 0 ? "check + update only (CPU only)" : "update + 50 us booking-record write (I/O wait)", "Locking scheme", "Time (ms)", "Bookings/s", "Booked", "double-booking?");
        for (int g = 0; g < 3; g++) {
            double best = 1e9; long ok = 0; int taken = 0;
            for (int rep = 0; rep < 3; rep++) { double t = run(g, ios[k], &ok, &taken); if (t < best) best = t; }
            printf("%-26s %10.1f %14.0f %9ld %s\n", name[g], best * 1e3, THREADS * BOOKINGS / best, ok, ok == taken ? "none" : "YES");
        }
    }
    return 0;
}
