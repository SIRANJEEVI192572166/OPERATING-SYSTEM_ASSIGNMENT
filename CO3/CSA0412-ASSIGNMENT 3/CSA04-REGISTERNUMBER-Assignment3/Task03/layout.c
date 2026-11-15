#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int global_init = 7; int global_bss;
void code_fn(void) {}
int main(void) {
    int local; char *h = malloc(100000);
    printf("code  (text) : %p  main\n", (void *)code_fn);
    printf("data  (.data): %p  global_init\n", (void *)&global_init);
    printf("bss          : %p  global_bss\n", (void *)&global_bss);
    printf("heap         : %p  malloc'd\n", (void *)h);
    printf("stack        : %p  local\n\n", (void *)&local);
    char line[512]; FILE *f = fopen("/proc/self/maps", "r");
    printf("/proc/self/maps:\n");
    while (fgets(line, sizeof line, f)) { line[strcspn(line, "\n")] = 0; puts(line); }
    fclose(f); free(h); return 0;
}
