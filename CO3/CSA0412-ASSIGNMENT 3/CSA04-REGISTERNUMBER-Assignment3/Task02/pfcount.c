#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
int main(int argc, char **argv) {
    pid_t p = fork();
    if (p == 0) { freopen("/dev/null", "w", stdout); execvp(argv[1], argv + 1); _exit(127); }
    int st; struct rusage ru; wait4(p, &st, 0, &ru);
    printf("command: %s\nminor page faults: %ld\nmajor page faults: %ld\nmax RSS: %ld kB\n", argv[1], ru.ru_minflt, ru.ru_majflt, ru.ru_maxrss);
    return 0;
}
