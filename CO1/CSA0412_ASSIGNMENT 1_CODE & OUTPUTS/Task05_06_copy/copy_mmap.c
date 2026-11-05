#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
int main(int argc, char **argv)
{
    int in = open(argv[1], O_RDONLY), out = open("mmap.out", O_RDWR|O_CREAT|O_TRUNC, 0644);
    struct stat st; fstat(in, &st);
    ftruncate(out, st.st_size);
    char *s = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, in, 0);
    char *d = mmap(NULL, st.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, out, 0);
    memcpy(d, s, st.st_size);
    munmap(s, st.st_size); munmap(d, st.st_size);
    close(in); close(out); unlink("mmap.out");
    printf("mmap copy: %lld bytes\n", (long long)st.st_size);
    return 0;
}
