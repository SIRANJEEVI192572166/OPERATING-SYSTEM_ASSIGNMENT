#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>

/* Print State and VmRSS of a process from /proc/<pid>/status */
static void show_proc(pid_t pid, const char *role)
{
    char path[64], line[256], state[64] = "?", rss[64] = "-";
    snprintf(path, sizeof path, "/proc/%d/status", (int)pid);
    FILE *f = fopen(path, "r");
    if (!f) { printf("  %-22s pid=%d  (no /proc entry)\n", role, (int)pid); return; }
    while (fgets(line, sizeof line, f)) {
        if (!strncmp(line, "State:", 6)) sscanf(line, "State:\t%63[^\n]", state);
        if (!strncmp(line, "VmRSS:", 6)) sscanf(line, "VmRSS:\t%63[^\n]", rss);
    }
    fclose(f);
    printf("  %-22s pid=%-6d State: %-14s VmRSS: %s\n", role, (int)pid, state, rss);
}

int main(void)
{
    pid_t kids[4];
    const char *role[4] = {"child A (sleeping)", "child B (CPU busy)",
                           "child C (sleeping)", "child D (exited early)"};
    printf("parent pid=%d\n", (int)getpid());
    for (int i = 0; i < 4; i++) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); return 1; }
        if (pid == 0) {                       /* ---- child ---- */
            if (i == 1) {                     /* busy loop => Running/Ready */
                volatile unsigned long x = 0;
                for (unsigned long k = 0; k < 4000000000UL; k++) x += k;
            } else if (i == 3) {
                _exit(7);                     /* terminates at once */
            } else {
                sleep(3);                     /* blocked => Waiting */
            }
            _exit(i);
        }
        kids[i] = pid;                        /* ---- parent ---- */
    }
    nanosleep(&(struct timespec){0, 500000000L}, NULL);   /* let the children settle */
    printf("\n/proc evidence (D has exited but is not yet reaped):\n");
    show_proc(getpid(), "parent");
    for (int i = 0; i < 4; i++) show_proc(kids[i], role[i]);

    printf("\nps evidence:\n");
    char cmd[128];
    snprintf(cmd, sizeof cmd, "ps -o pid,ppid,stat,wchan:14,etime,comm --ppid %d", (int)getpid());
    fflush(stdout);
    if (system(cmd) == -1) perror("system");

    printf("\nReaping children with waitpid():\n");
    for (int i = 0; i < 4; i++) {
        int st;
        if (i == 1) kill(kids[i], SIGKILL);   /* stop the busy child early */
        waitpid(kids[i], &st, 0);
        if (WIFEXITED(st)) printf("  pid %d terminated, exit status %d\n", (int)kids[i], WEXITSTATUS(st));
        else printf("  pid %d terminated by signal %d\n", (int)kids[i], WTERMSIG(st));
    }
    return 0;
}
