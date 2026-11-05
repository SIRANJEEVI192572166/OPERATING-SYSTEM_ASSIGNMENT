#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <time.h>
static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec*1e9+t.tv_nsec; }
__attribute__((noinline)) long fcall(long x){ __asm__ volatile("" : "+r"(x)); return x+1; }
int main(void)
{
    long N = 2000000, s = 0; double t;
    t = now(); for (long i = 0; i < N; i++) s += fcall(i); double f = (now()-t)/N;
    t = now(); for (long i = 0; i < N; i++) s += syscall(SYS_getpid);  double sc = (now()-t)/N;
    /* message passing between two processes through pipes: round trip */
    int p1[2], p2[2]; pipe(p1); pipe(p2); char c = 'x'; long R = 100000;
    if (fork() == 0) { for (long i = 0; i < R; i++) { read(p1[0], &c, 1); write(p2[1], &c, 1); } _exit(0); }
    t = now(); for (long i = 0; i < R; i++) { write(p1[1], &c, 1); read(p2[0], &c, 1); } double ipc = (now()-t)/R;
    wait(NULL);
    printf("ordinary function call        : %8.1f ns\n", f);
    printf("system call (getpid)          : %8.1f ns\n", sc);
    printf("IPC round trip (2 pipes)      : %8.1f ns\n", ipc);
    printf("system call / function call   : %8.1f x\n", sc/f);
    printf("IPC round trip / system call  : %8.1f x\n", ipc/sc);
    return (int)(s & 0);
}
