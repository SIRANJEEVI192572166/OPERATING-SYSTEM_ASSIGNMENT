#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#define SLOTS 4
#define END (-1)                          /* sentinel reading: no more data */

struct shared { int buf[SLOTS]; int in, out; sem_t empty, full; };

int main(void)
{
    struct shared *s = mmap(NULL, sizeof *s, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (s == MAP_FAILED) { perror("mmap"); return 1; }
    sem_init(&s->empty, 1, SLOTS);        /* pshared = 1: usable between processes */
    sem_init(&s->full, 1, 0);
    s->in = s->out = 0;
    pid_t pid = fork();
    if (pid == 0) {                        /* analyser: consumer */
        int count = 0, sum = 0, max = 0;
        for (;;) {
            sem_wait(&s->full);
            int x = s->buf[s->out]; s->out = (s->out + 1) % SLOTS;
            sem_post(&s->empty);
            if (x == END) break;
            count++; sum += x; if (x > max) max = x;
        }
        printf("analyser: %d readings, average %.1f, max %d\n", count, count ? (double)sum / count : 0.0, max);
        fflush(stdout); _exit(0);
    }
    int readings[] = {31, 33, 36, 34, 41, 38, END};   /* sensor reader: producer */
    for (int i = 0; i < 7; i++) {
        sem_wait(&s->empty);
        s->buf[s->in] = readings[i]; s->in = (s->in + 1) % SLOTS;
        sem_post(&s->full);
    }
    waitpid(pid, NULL, 0);
    printf("reader: done\n");
    sem_destroy(&s->empty); sem_destroy(&s->full); munmap(s, sizeof *s);
    return 0;
}
