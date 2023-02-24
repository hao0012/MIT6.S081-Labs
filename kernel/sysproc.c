#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;
  backtrace();
  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 sys_sigalarm(void) {
  struct proc *p = myproc();
  argint(0, &(p->ticks));
  uint64 addr;
  argaddr(1, &addr);
  p->handler = (void (*)(void)) addr;
  p->ticks_left = p->ticks;
  return 0;
}

static void recover(struct proc *p) {
  struct trapframe* c = p->trapframe, *o = p->tf_copy;
  c->a0 = o->a0;
  c->a1 = o->a1;
  c->a2 = o->a2;
  c->a3 = o->a3;
  c->a4 = o->a4;
  c->a5 = o->a5;
  c->a6 = o->a6;
  c->a7 = o->a7;

  c->epc = o->epc;
  c->gp = o->gp;
  c->ra = o->ra;
  
  c->s0 = o->s0;
  c->s1 = o->s1;
  c->s2 = o->s2;
  c->s3 = o->s3;
  c->s4 = o->s4;
  c->s5 = o->s5;
  c->s6 = o->s6;
  c->s7 = o->s7;
  c->s8 = o->s8;
  c->s9 = o->s9;
  c->s10 = o->s10;
  c->s11 = o->s11;

  c->sp = o->sp;
  
  c->t0 = o->t0;
  c->t1 = o->t1;
  c->t2 = o->t2;
  c->t3 = o->t3;
  c->t4 = o->t4;
  c->t5 = o->t5;
  c->t6 = o->t6;

  c->tp = o->tp;
}

uint64 sys_sigreturn(void) {
  struct proc *p = myproc();
  recover(p);
  p->handling = 0;
  return 0;
}