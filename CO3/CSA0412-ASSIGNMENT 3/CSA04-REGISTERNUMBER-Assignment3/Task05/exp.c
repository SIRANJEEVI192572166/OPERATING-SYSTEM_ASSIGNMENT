#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N 100000
#define PAGES 100
#define MAXF 16
static int ref[N], nxt[N];
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
static int find(const int *f,int n,int p){for(int i=0;i<n;i++)if(f[i]==p)return i;return -1;}
static int fifo(int fr){int f[MAXF],u=0,nx=0,c=0;for(int i=0;i<N;i++){if(find(f,u,ref[i])>=0)continue;c++;if(u<fr)f[u++]=ref[i];else{f[nx]=ref[i];nx=(nx+1)%fr;}}return c;}
static int lru(int fr){int f[MAXF],l[MAXF],u=0,c=0;for(int i=0;i<N;i++){int k=find(f,u,ref[i]);if(k>=0){l[k]=i;continue;}c++;if(u<fr){f[u]=ref[i];l[u]=i;u++;}else{int v=0;for(int j=1;j<fr;j++)if(l[j]<l[v])v=j;f[v]=ref[i];l[v]=i;}}return c;}
static int clk(int fr){int f[MAXF],b[MAXF],u=0,h=0,c=0;for(int i=0;i<N;i++){int k=find(f,u,ref[i]);if(k>=0){b[k]=1;continue;}c++;if(u<fr){f[u]=ref[i];b[u]=1;u++;continue;}while(b[h]){b[h]=0;h=(h+1)%fr;}f[h]=ref[i];b[h]=1;h=(h+1)%fr;}return c;}
static int opt(int fr){int f[MAXF],nu[MAXF],u=0,c=0;for(int i=0;i<N;i++){int k=find(f,u,ref[i]);if(k>=0){nu[k]=nxt[i];continue;}c++;if(u<fr){f[u]=ref[i];nu[u]=nxt[i];u++;}else{int v=0;for(int j=1;j<fr;j++)if(nu[j]>nu[v])v=j;f[v]=ref[i];nu[v]=nxt[i];}}return c;}
static void gen(int locality){srand(12345);int ws[8];for(int j=0;j<8;j++)ws[j]=rand()%PAGES;
  for(int i=0;i<N;i++){ if(locality){ if(i%2000==0)for(int j=0;j<8;j++)ws[j]=rand()%PAGES; ref[i]=(rand()%100<90)?ws[rand()%8]:rand()%PAGES; } else ref[i]=rand()%PAGES; }
  int last[PAGES];for(int p=0;p<PAGES;p++)last[p]=N;for(int i=N-1;i>=0;i--){nxt[i]=last[ref[i]];last[ref[i]]=i;}}
int main(void){
  const char *nm[2]={"random","locality"};
  printf("workload,frames,FIFO,LRU,CLOCK,OPT\n");
  double tt[2][4]={{0}};
  for(int w=0;w<2;w++){gen(w);
    for(int fr=3;fr<=10;fr++){double t;int a,b,c,d;
      t=now();a=fifo(fr);tt[w][0]+=now()-t; t=now();b=lru(fr);tt[w][1]+=now()-t; t=now();c=clk(fr);tt[w][2]+=now()-t; t=now();d=opt(fr);tt[w][3]+=now()-t;
      printf("%s,%d,%d,%d,%d,%d\n",nm[w],fr,a,b,c,d);}}
  fprintf(stderr,"TIME total for frames 3..10 (8 runs x 100,000 refs), seconds\n%-9s %9s %9s %9s %9s\n","workload","FIFO","LRU","CLOCK","OPT");
  for(int w=0;w<2;w++)fprintf(stderr,"%-9s %9.4f %9.4f %9.4f %9.4f\n",nm[w],tt[w][0],tt[w][1],tt[w][2],tt[w][3]);
  return 0;}
