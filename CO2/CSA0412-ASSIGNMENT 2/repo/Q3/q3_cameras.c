#include "work.h"
#include <pthread.h>
#include <stdatomic.h>

static atomic_int next_cam;
static int total_plates;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static void *one_camera(void *arg)               /* thread-per-camera */
{
    int found = process_frame((int)(long)arg);
    pthread_mutex_lock(&lock); total_plates += found; pthread_mutex_unlock(&lock);
    return NULL;
}
static void *pool_worker(void *arg)              /* pool: take the next camera until none left */
{
    (void)arg; int local = 0, c;
    while ((c = atomic_fetch_add(&next_cam, 1)) < CAMERAS) local += process_frame(c);
    pthread_mutex_lock(&lock); total_plates += local; pthread_mutex_unlock(&lock);
    return NULL;
}
static double run(int mode, int nthreads, int *plates)   /* mode 0 sequential, 1 per-camera, 2 pool */
{
    total_plates = 0; atomic_store(&next_cam, 0);
    double t0 = now();
    if (mode == 0) for (int c = 0; c < CAMERAS; c++) total_plates += process_frame(c);
    else if (mode == 1) {
        pthread_t th[CAMERAS];
        for (long c = 0; c < CAMERAS; c++) pthread_create(&th[c], NULL, one_camera, (void *)c);
        for (int c = 0; c < CAMERAS; c++) pthread_join(th[c], NULL);
    } else {
        pthread_t th[64];
        for (int i = 0; i < nthreads; i++) pthread_create(&th[i], NULL, pool_worker, NULL);
        for (int i = 0; i < nthreads; i++) pthread_join(th[i], NULL);
    }
    *plates = total_plates; return now() - t0;
}
int main(void)
{
    printf("CPUs online: %ld, cameras: %d, per-frame: 2 ms I/O wait + CPU work\n\n", sysconf(_SC_NPROCESSORS_ONLN), CAMERAS);
    printf("%-22s %10s %9s %8s\n", "Version", "Time (ms)", "Speed-up", "Plates");
    int plates; double base = 0;
    struct { const char *name; int mode, n; } cfg[] = {
        {"Sequential (1 thread)", 0, 1}, {"Pool of 4 threads", 2, 4}, {"Pool of 8 threads", 2, 8},
        {"Pool of 16 threads", 2, 16}, {"Thread per camera (64)", 1, 64} };
    for (int k = 0; k < 5; k++) {
        double best = 1e9; for (int rep = 0; rep < 3; rep++) { double t = run(cfg[k].mode, cfg[k].n, &plates); if (t < best) best = t; }
        if (k == 0) base = best;
        printf("%-22s %10.1f %9.2f %8d\n", cfg[k].name, best * 1e3, base / best, plates);
    }
    return 0;
}
