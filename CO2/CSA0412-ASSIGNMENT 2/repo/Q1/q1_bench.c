#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <sys/wait.h>

static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec*1e-9; }
static long avail_kb(void){
    FILE *f=fopen("/proc/meminfo","r"); char l[128]; long v=0;
    while(fgets(l,sizeof l,f)) if(!strncmp(l,"MemAvailable:",13)) sscanf(l,"MemAvailable: %ld",&v);
    fclose(f); return v;
}
static void *sleeper(void *a){ (void)a; pause(); return NULL; }

static void pin(void){ cpu_set_t s; CPU_ZERO(&s); CPU_SET(0,&s); sched_setaffinity(0,sizeof s,&s); }

#define ROUNDS 100000
static int p1[2], p2[2];
static void *pong(void *a){ (void)a; char c; for(int i=0;i<ROUNDS;i++){ if(read(p1[0],&c,1)!=1)break; if(write(p2[1],&c,1)<0) break;} return NULL; }

int main(int argc, char **argv)
{
    int N = argc > 1 ? atoi(argv[1]) : 2000;
    pin();
    printf("Active sessions simulated: %d\n\n", N);

    /* ---- N processes ---- */
    pid_t *pids = malloc(N*sizeof(pid_t));
    long m0 = avail_kb(); double t0 = now(); int made = 0;
    for (int i=0;i<N;i++){ pid_t p=fork(); if(p<0){perror("fork");break;} if(p==0){pause();_exit(0);} pids[i]=p; made++; }
    double tproc = now()-t0; usleep(300000); long m1 = avail_kb();
    printf("PROCESSES: created %d in %.1f ms (%.1f us each); memory used %ld KB (%.1f KB each)\n",
           made, tproc*1e3, tproc*1e6/made, m0-m1, (double)(m0-m1)/made);
    for(int i=0;i<made;i++) kill(pids[i],SIGKILL);
    for(int i=0;i<made;i++) waitpid(pids[i],NULL,0);
    usleep(300000);

    /* ---- N threads (64 KB stacks) ---- */
    pthread_t *th = malloc(N*sizeof(pthread_t)); pthread_attr_t at; pthread_attr_init(&at);
    pthread_attr_setstacksize(&at, 64*1024);
    m0 = avail_kb(); t0 = now(); made = 0;
    for (int i=0;i<N;i++){ if(pthread_create(&th[i],&at,sleeper,NULL)){perror("pthread_create");break;} made++; }
    double tthr = now()-t0; usleep(300000); m1 = avail_kb();
    printf("THREADS:   created %d in %.1f ms (%.1f us each); memory used %ld KB (%.1f KB each)\n",
           made, tthr*1e3, tthr*1e6/made, m0-m1, (double)(m0-m1)/made);
    for(int i=0;i<made;i++) pthread_cancel(th[i]);
    for(int i=0;i<made;i++) pthread_join(th[i],NULL);

    /* ---- context-switch cost: pipe ping-pong pinned to one CPU ---- */
    if(pipe(p1)||pipe(p2)) return 1; char c='x';
    pid_t pid = fork();
    if(pid==0){ pin(); pong(NULL); _exit(0);} 
    t0=now(); for(int i=0;i<ROUNDS;i++){ if(write(p1[1],&c,1)<0||read(p2[0],&c,1)<0) return 1;} double tp=now()-t0; waitpid(pid,NULL,0);
    pthread_t t; pthread_create(&t,NULL,pong,NULL);
    t0=now(); for(int i=0;i<ROUNDS;i++){ if(write(p1[1],&c,1)<0||read(p2[0],&c,1)<0) return 1;} double tt=now()-t0; pthread_join(t,NULL);
    printf("\nCONTEXT SWITCH (pipe ping-pong, %d round trips, 1 CPU):\n", ROUNDS);
    printf("  process <-> process : %.2f us per switch\n", tp*1e6/(2.0*ROUNDS));
    printf("  thread  <-> thread  : %.2f us per switch\n", tt*1e6/(2.0*ROUNDS));
    return 0;
}
