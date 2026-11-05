#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <spawn.h>
#include <sys/wait.h>
#include <time.h>
extern char **environ;
static double ms(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec*1e3+t.tv_nsec/1e6; }
static void *nop(void *a){ return a; }
int main(int argc, char **argv)
{
    int N = 1000; double t;
    if (argc < 2) return 1;
    if (!strcmp(argv[1], "fork")) {
        t = ms(); for (int i = 0; i < N; i++) { pid_t p = fork(); if (p == 0) _exit(0); waitpid(p, NULL, 0); }
        printf("1000 x fork()+exit+wait         : %8.1f ms\n", ms()-t);
    } else if (!strcmp(argv[1], "forkbig")) {
        size_t sz = 200u << 20; char *m = malloc(sz); memset(m, 1, sz);   /* 200 MB touched */
        t = ms(); for (int i = 0; i < N; i++) { pid_t p = fork(); if (p == 0) _exit(0); waitpid(p, NULL, 0); }
        printf("1000 x fork() of 200 MB process : %8.1f ms\n", ms()-t);
    } else if (!strcmp(argv[1], "thread")) {
        t = ms(); for (int i = 0; i < N; i++) { pthread_t th; pthread_create(&th, NULL, nop, NULL); pthread_join(th, NULL); }
        printf("1000 x pthread_create+join      : %8.1f ms\n", ms()-t);
    } else if (!strcmp(argv[1], "forkexec")) {
        t = ms(); for (int i = 0; i < N; i++) { pid_t p = fork(); if (p == 0) { execl("/bin/true","true",(char*)NULL); _exit(127);} waitpid(p, NULL, 0); }
        printf("1000 x fork()+exec(true)+wait   : %8.1f ms\n", ms()-t);
    } else if (!strcmp(argv[1], "vforkexec")) {
        t = ms(); for (int i = 0; i < N; i++) { pid_t p = vfork(); if (p == 0) { execl("/bin/true","true",(char*)NULL); _exit(127);} waitpid(p, NULL, 0); }
        printf("1000 x vfork()+exec(true)+wait  : %8.1f ms\n", ms()-t);
    } else if (!strcmp(argv[1], "spawn")) {
        char *av[] = {"true", NULL};
        t = ms(); for (int i = 0; i < N; i++) { pid_t p; posix_spawn(&p, "/bin/true", NULL, NULL, av, environ); waitpid(p, NULL, 0); }
        printf("1000 x posix_spawn(true)+wait   : %8.1f ms\n", ms()-t);
    }
    return 0;
}
