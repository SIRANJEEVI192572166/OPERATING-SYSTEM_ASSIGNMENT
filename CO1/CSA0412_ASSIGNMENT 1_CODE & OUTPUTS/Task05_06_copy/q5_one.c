#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

/* Copy src to dst with the read() and write() system calls, using a buffer of
   bufsize bytes. Returns the number of read() calls made, or -1 on error. */
long copy_file(const char *src, const char *dst, size_t bufsize)
{
    char buf[8192];
    int in = open(src, O_RDONLY);
    if (in < 0) return -1;
    int out = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out < 0) { close(in); return -1; }
    long calls = 0;
    ssize_t n;
    // TODO 1: Loop: read up to bufsize bytes into buf (count every read call in calls);
    //   stop when read returns 0 (end of file) or -1; write exactly the n bytes that
    //   were read
    if (bufsize > sizeof buf) bufsize = sizeof buf;      /* never overflow buf */
    while ((n = read(in, buf, bufsize)) > 0) {
        calls++;                                          /* count every successful read */
        ssize_t done = 0;
        while (done < n) {                                /* write exactly n bytes */
            ssize_t w = write(out, buf + done, (size_t)(n - done));
            if (w < 0) { close(in); close(out); return -1; }
            done += w;
        }
    }
    calls++;                                              /* the final read that returned 0 or -1 */
    if (n < 0) calls = -1;                                /* read error */

    close(in);
    close(out);
    return calls;
}

int main(int argc, char **argv)
{
    FILE *f = fopen("records.dat", "w");
    for (int i = 0; i < 10000; i++) fputc('A' + i % 26, f);
    fclose(f);
    size_t b = (size_t)atol(argv[1]);
    fprintf(stderr, "program reports: buffer %4zu bytes: %5ld read() calls\n", b, copy_file("records.dat", "copy.dat", b));
    unlink("records.dat"); unlink("copy.dat");
    return 0;
}
