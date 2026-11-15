#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Units: 1 page-unit = 4 MB.  RAM 2048 MB, kernel+OS 300 MB -> 437 units for tasks. */
#define FR 437
enum {RT, ML, LOG, EXTRA, NT};
static const char *tn[NT] = {"RT control", "ML", "Logging", "4th task"};
static int size[NT] = {25, 375, 13, 125};   /* 100 MB, 1500 MB, ~50 MB, 500 MB */
static int base[NT];
typedef struct { int page, last, ref, pin, valid; } Fr;
static Fr fr[FR]; static int hand;
static long fault[NT];
static int find(int p){ for (int i=0;i<FR;i++) if (fr[i].valid && fr[i].page==p) return i; return -1; }
static int victim(int pol, long clk) {
    int v=-1;
    for (int i=0;i<FR;i++) if (!fr[i].valid) return i;
    if (pol==0) { for (int k=0;k<FR;k++){ int i=(hand+k)%FR; if(!fr[i].pin){ hand=(i+1)%FR; return i; } } }
    if (pol==1) { for (int i=0;i<FR;i++) if(!fr[i].pin && (v<0 || fr[i].last<fr[v].last)) v=i; return v; }
    for (;;) { int i=hand; hand=(hand+1)%FR; if(fr[i].pin) continue; if(fr[i].ref){fr[i].ref=0;continue;} return i; }
    (void)clk;
}
static void run(int pol, int ntasks, int pin_rt) {
    memset(fr,0,sizeof fr); memset(fault,0,sizeof fault); hand=0; int b=0;
    for (int t=0;t<NT;t++){ base[t]=b; b+=size[t]; }
    long clk=0; srand(5); int mlpos=0, logpos=0;
    for (int tick=0; tick<4000; tick++) {
        int refs[64], who[64], n=0;
        for (int i=0;i<25;i++){ refs[n]=base[RT]+i; who[n++]=RT; }                       /* RT loop touches all its pages each tick */
        for (int i=0;i<25;i++){ refs[n]=base[ML]+mlpos; who[n++]=ML; mlpos=(mlpos+1)%size[ML]; } /* ML sweeps its model cyclically */
        refs[n]=base[LOG]+logpos; who[n++]=LOG; logpos=(logpos+1)%size[LOG];
        if (ntasks==4) for (int i=0;i<10;i++){ refs[n]=base[EXTRA]+rand()%size[EXTRA]; who[n++]=EXTRA; }
        for (int k=0;k<n;k++) {
            clk++; int i=find(refs[k]);
            if (i>=0) { fr[i].last=clk; fr[i].ref=1; continue; }
            fault[who[k]]++;
            int v=victim(pol,clk);
            fr[v]=(Fr){refs[k],clk,1,(who[k]==RT&&pin_rt),1};
        }
    }
}
int main(void) {
    const char *pn[3]={"FIFO","LRU","Clock"};
    printf("Page faults after 4000 ticks (1 unit = 4 MB, 437 units available)\n");
    printf("%-34s %-6s %10s %10s %10s %10s\n","Scenario","Policy",tn[0],tn[1],tn[2],tn[3]);
    int cases[3][3]={{3,0,1},{4,0,0},{4,0,1}}; const char *cn[3]={"3 tasks, RT pages locked","4 tasks, RT NOT locked","4 tasks, RT pages locked"};
    for (int c=0;c<3;c++) for (int p=0;p<3;p++) {
        run(p,cases[c][0]==3?3:4,cases[c][2]);
        printf("%-34s %-6s %10ld %10ld %10ld %10ld\n",cn[c],pn[p],fault[0],fault[1],fault[2],fault[3]);
    }
    return 0;
}
