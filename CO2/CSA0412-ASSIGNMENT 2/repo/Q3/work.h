#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#define CAMERAS 64
#define IO_NS   2000000L      /* 2 ms: waiting for the frame from the camera (I/O)  */
#define CPU_ITERS 1000000UL   /* number-plate recognition: CPU-bound work per frame */

static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec*1e-9; }

/* Reads one frame (I/O wait) then "recognises" plates (CPU). Returns plates found (deterministic). */
static int process_frame(int cam)
{
    nanosleep(&(struct timespec){0, IO_NS}, NULL);
    unsigned long h = 2166136261UL + (unsigned long)cam;
    for (unsigned long i = 0; i < CPU_ITERS; i++) h = (h ^ i) * 16777619UL;
    return (int)(h % 5) + 1;               /* 1..5 plates in this frame */
}
