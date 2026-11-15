#include <stdio.h>
struct seg { const char *name; unsigned base, limit; const char *perm; };
struct seg table[] = {
    {"code ", 1400, 1000, "R-X"}, {"data ", 6300, 400, "RW-"}, {"stack", 4300, 400, "RW-"},
    {"heap ", 3200, 1100, "RW-"}, {"const", 4700, 1000, "R--"}};
#define NSEG 5
static void translate(unsigned s, unsigned d, char op) {
    printf("(seg %u, off %4u, %c) -> ", s, d, op);
    if (s >= NSEG) { printf("TRAP: invalid segment number\n"); return; }
    if (d >= table[s].limit) { printf("TRAP: offset %u >= limit %u (segmentation fault)\n", d, table[s].limit); return; }
    if (op == 'W' && table[s].perm[1] != 'W') { printf("TRAP: write to %s segment (protection fault)\n", table[s].perm); return; }
    printf("%-5s physical = %u + %u = %u\n", table[s].name, table[s].base, d, table[s].base + d);
}
int main(void) {
    printf("Segment table: no name  base limit perm\n");
    for (int i = 0; i < NSEG; i++) printf("               %d  %s %4u %4u %s\n", i, table[i].name, table[i].base, table[i].limit, table[i].perm);
    printf("\n");
    unsigned t[8][2] = {{0,0},{0,999},{1,399},{2,200},{3,1099},{4,500},{1,400},{3,1100}};
    char op[8] = {'R','R','W','W','W','R','R','R'};
    for (int i = 0; i < 8; i++) translate(t[i][0], t[i][1], op[i]);
    translate(0, 100, 'W'); translate(4, 10, 'W'); translate(5, 0, 'R');
    return 0;
}
