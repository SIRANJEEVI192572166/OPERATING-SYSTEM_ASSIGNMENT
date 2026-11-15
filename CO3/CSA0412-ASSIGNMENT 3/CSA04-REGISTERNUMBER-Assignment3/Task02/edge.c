#include <stdio.h>
#define PAGE_SIZE 4096
#define PAGES 8
int page_table[PAGES] = {5, 2, -1, 7, 0, -1, 3, -1};
int main(void) {
    unsigned t[] = {0, 4095, 4096, 8191, 12288, 28671, 28672, 32767, 32768};
    const char *note[] = {"start of page 0","end of page 0","start of page 1","end of page 1",
        "start of page 3","end of page 6","start of page 7 (not resident)","end of page 7","beyond last page"};
    for (int i = 0; i < 9; i++) {
        unsigned p = t[i] / PAGE_SIZE, d = t[i] % PAGE_SIZE;
        printf("%6u (%-30s) page %u off %4u : ", t[i], note[i], p, d);
        if (p >= PAGES) puts("invalid address (trap to OS)");
        else if (page_table[p] < 0) puts("page fault");
        else printf("physical %u\n", (unsigned)page_table[p]*PAGE_SIZE + d);
    }
    return 0;
}
