/* sc: minimal "strace -c" style syscall counter using ptrace (strace is not installed here). */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <time.h>
#define MAXN 512
static long cnt[MAXN]; static long errs[MAXN]; static double tm[MAXN];
static const char *name(long n){
  switch(n){
#define N(x) case SYS_##x: return #x;
  N(read) N(write) N(open) N(openat) N(close) N(fstat) N(newfstatat) N(mmap) N(munmap) N(mprotect) N(brk)
  N(execve) N(arch_prctl) N(access) N(exit_group) N(set_tid_address) N(set_robust_list) N(rseq) N(prlimit64)
  N(getrandom) N(pread64) N(unlink) N(clone) N(clone3) N(wait4) N(lseek) N(futex) N(rt_sigaction) N(rt_sigprocmask)
  N(statfs) N(ioctl) N(getpid) N(fork) N(vfork) N(madvise) N(readlink) N(sched_getaffinity) N(sigaltstack) N(exit)
  N(gettid) N(tgkill) N(getpgrp) N(fadvise64) N(copy_file_range) N(sendfile) N(unlinkat)
  default: return NULL; }
}
static double now(){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
static void peekstr(pid_t p,unsigned long a,char*o,int n){int i=0;while(i<n-1){long w=ptrace(PTRACE_PEEKDATA,p,(void*)(a+i),0);for(int k=0;k<8&&i<n-1;k++,i++){char c=(w>>(8*k))&0xff;o[i]=c;if(!c)return;}}o[i]=0;}
int main(int argc,char**argv){
  setvbuf(stdout,NULL,_IOLBF,0); int trace=0; if(argc>1&&!strcmp(argv[1],"-t")){trace=1;argv++;argc--;}
  if(argc<2){fprintf(stderr,"usage: sc [-t] cmd...\n");return 2;}
  pid_t p=fork();
  if(p==0){ptrace(PTRACE_TRACEME,0,0,0);execvp(argv[1],argv+1);_exit(127);}
  int st; waitpid(p,&st,0);
  ptrace(PTRACE_SETOPTIONS,p,0,PTRACE_O_TRACESYSGOOD|PTRACE_O_TRACEFORK|PTRACE_O_TRACEVFORK|PTRACE_O_TRACECLONE);
  cnt[SYS_execve]=1; unsigned long a0=0,a1=0,a2=0; int in=0; long cur=-1; double t0=0; pid_t lastp=p;
  while(1){
    ptrace(PTRACE_SYSCALL,p,0,0);
    pid_t w=waitpid(-1,&st,__WALL); if(w<0)break;
    p=w;
    if(WIFEXITED(st)||WIFSIGNALED(st)){ if(w==lastp)break; continue; }
    if(WIFSTOPPED(st)&&(WSTOPSIG(st)==(SIGTRAP|0x80))){
      struct user_regs_struct r; ptrace(PTRACE_GETREGS,p,0,&r);
      /* entry vs exit tracked per stop: rax==-ENOSYS at entry */
      if((long)r.rax==-38){cur=r.orig_rax;t0=now();a0=r.rdi;a1=r.rsi;a2=r.rdx; if(cur==SYS_exit_group){cnt[cur]++;}}
      else { long n=r.orig_rax;
        if(trace&&(n==SYS_read||n==SYS_write||n==SYS_openat||n==SYS_close||n==SYS_unlink)){
          char b[256]; const char*nm=name(n);
          if(n==SYS_openat){peekstr(p,r.rsi?a1:a1,b,256);printf("openat(AT_FDCWD, \"%s\", 0x%lx) = %ld\n",b,a2,(long)r.rax);}
          else if(n==SYS_unlink){peekstr(p,a0,b,256);printf("unlink(\"%s\") = %ld\n",b,(long)r.rax);}
          else if(n==SYS_close)printf("close(%ld) = %ld\n",(long)a0,(long)r.rax);
          else printf("%s(%ld, \"...\", %lu) = %ld\n",nm,(long)a0,a2,(long)r.rax);} if(n>=0&&n<MAXN){cnt[n]++;tm[n]+=now()-t0; if((long)r.rax<0&&(long)r.rax>-4096)errs[n]++;} }
    }
  }
  double tot=0;long ct=0,ce=0;
  for(int i=0;i<MAXN;i++){tot+=tm[i];ct+=cnt[i];ce+=errs[i];}
  fprintf(stderr,"%% time     seconds  usecs/call     calls    errors syscall\n");
  fprintf(stderr,"------ ----------- ----------- --------- --------- ----------------\n");
  /* sort desc by time */
  int idx[MAXN];int m=0;for(int i=0;i<MAXN;i++)if(cnt[i])idx[m++]=i;
  for(int a=0;a<m;a++)for(int b=a+1;b<m;b++)if(tm[idx[b]]>tm[idx[a]]){int t=idx[a];idx[a]=idx[b];idx[b]=t;}
  for(int k=0;k<m;k++){int i=idx[k];const char*nm=name(i);char buf[24];if(!nm){snprintf(buf,24,"syscall_%d",i);nm=buf;}
    fprintf(stderr,"%6.2f %11.6f %11.0f %9ld %9s %s\n",tot?100*tm[i]/tot:0,tm[i],cnt[i]?1e6*tm[i]/cnt[i]:0,cnt[i],errs[i]?(snprintf(buf,24,"%ld",errs[i]),strdup(buf)):"",nm);}
  fprintf(stderr,"------ ----------- ----------- --------- --------- ----------------\n");
  fprintf(stderr,"100.00 %11.6f %21ld %9ld total\n",tot,ct,ce);
  return 0;}
