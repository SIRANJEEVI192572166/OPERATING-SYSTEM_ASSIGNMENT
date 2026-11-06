#include "work.h"
#include <sys/wait.h>
#include <string.h>

/* nworkers processes; worker w handles cameras w, w+nworkers, ... and writes its plate count to a pipe */
static double run(int nworkers, int *plates)
{
    int fd[2]; if (pipe(fd)) { perror("pipe"); exit(1); }
    double t0 = now();
    for (int w = 0; w < nworkers; w++) {
        pid_t pid = fork();
        if (pid == 0) {
            int local = 0;
            for (int c = w; c < CAMERAS; c += nworkers) local += process_frame(c);
            if (write(fd[1], &local, sizeof local) < 0) _exit(1);       /* IPC: result goes through the kernel */
            _exit(0);
        }
    }
    close(fd[1]);
    int total = 0, part;
    while (read(fd[0], &part, sizeof part) == sizeof part) total += part;   /* combine results */
    while (wait(NULL) > 0) ;
    close(fd[0]);
    *plates = total; return now() - t0;
}
int main(void)
{
    printf("Multi-process version (CPUs online: %ld)\n%-24s %10s %9s %8s\n", sysconf(_SC_NPROCESSORS_ONLN),
           "Version", "Time (ms)", "Speed-up", "Plates");
    int counts[] = {1, 4, 8, 16, 64}, plates; double base = 0;
    for (int k = 0; k < 5; k++) {
        double best = 1e9; for (int rep = 0; rep < 3; rep++) { double t = run(counts[k], &plates); if (t < best) best = t; }
        if (k == 0) base = best;
        char name[40]; snprintf(name, sizeof name, "%d process%s", counts[k], counts[k] == 1 ? "" : "es");
        printf("%-24s %10.1f %9.2f %8d\n", name, best * 1e3, base / best, plates);
    }
    return 0;
}
