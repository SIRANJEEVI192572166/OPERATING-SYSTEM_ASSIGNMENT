#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#define CHUNK (64 * 1024)
#define SLOTS 8

static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec*1e-9; }
static unsigned long sum_bytes(const unsigned char *p, size_t n){ unsigned long s=0; for(size_t i=0;i<n;i++) s+=p[i]; return s; }

/* Pipe: producer fills a private buffer, write() copies it into the kernel, reader's read() copies it out again */
static double via_pipe(size_t total, unsigned long *chk)
{
    int fd[2]; if (pipe(fd)) exit(1);
    int rp[2]; if (pipe(rp)) exit(1);            /* result pipe: child -> parent */
    double t0 = now(); pid_t pid = fork();
    if (pid == 0) {
        close(fd[1]); close(rp[0]); unsigned char *b = malloc(CHUNK); unsigned long s = 0; ssize_t n;
        while ((n = read(fd[0], b, CHUNK)) > 0) s += sum_bytes(b, (size_t)n);
        if (write(rp[1], &s, sizeof s) < 0) { _exit(1); }
        _exit(0);
    }
    close(fd[0]); close(rp[1]); unsigned char *b = malloc(CHUNK);
    for (size_t sent = 0; sent < total; sent += CHUNK) { memset(b, (int)(sent / CHUNK) & 0xff, CHUNK); if (write(fd[1], b, CHUNK) < 0) exit(1); }
    close(fd[1]); if (read(rp[0], chk, sizeof *chk) < 0) exit(1); waitpid(pid, NULL, 0);
    close(fd[0]); close(rp[0]); free(b); return now() - t0;
}
/* Shared memory ring: no data goes through the kernel; semaphores provide the synchronisation */
struct ring { unsigned char slot[SLOTS][CHUNK]; sem_t empty, full; unsigned long result; };
static double via_shm(size_t total, unsigned long *chk)
{
    struct ring *r = mmap(NULL, sizeof *r, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    sem_init(&r->empty, 1, SLOTS); sem_init(&r->full, 1, 0);
    size_t chunks = total / CHUNK; double t0 = now(); pid_t pid = fork();
    if (pid == 0) { unsigned long s = 0; for (size_t k = 0; k < chunks; k++) { sem_wait(&r->full); s += sum_bytes(r->slot[k % SLOTS], CHUNK); sem_post(&r->empty); } r->result = s; _exit(0); }
    for (size_t k = 0; k < chunks; k++) { sem_wait(&r->empty); memset(r->slot[k % SLOTS], (int)k & 0xff, CHUNK); sem_post(&r->full); }
    waitpid(pid, NULL, 0); *chk = r->result; double t = now() - t0; munmap(r, sizeof *r); return t;
}
int main(void)
{
    size_t mb[3] = {1, 10, 100};
    printf("%-8s %14s %14s %10s  %s\n", "Size", "Pipe (ms)", "Shared mem (ms)", "Pipe/Shm", "checksums equal");
    for (int k = 0; k < 3; k++) {
        size_t total = mb[k] * 1024 * 1024; double bp = 1e9, bs = 1e9; unsigned long cp = 0, cs = 0;
        for (int rep = 0; rep < 5; rep++) { double t = via_pipe(total, &cp); if (t < bp) bp = t; t = via_shm(total, &cs); if (t < bs) bs = t; }
        printf("%-4zu MB %14.2f %14.2f %10.2f  %s\n", mb[k], bp * 1e3, bs * 1e3, bp / bs, cp == cs ? "yes" : "NO");
    }
    return 0;
}
