#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
static const char ro[] = "read-only";
int main(void) {
    pid_t p = fork();
    if (p == 0) {
        printf("child: storing into read-only memory (a segment without write permission) ...\n");
        fflush(stdout);
        char *q = (char *)ro; q[0] = 'X';       /* store into the read-only segment */
        printf("not reached\n");
        return 0;
    }
    int st; waitpid(p, &st, 0);
    if (WIFSIGNALED(st)) printf("parent: child killed by signal %d (%s)\n", WTERMSIG(st), strsignal(WTERMSIG(st)));
    return 0;
}
