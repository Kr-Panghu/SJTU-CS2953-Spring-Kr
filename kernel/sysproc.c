#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
sys_wait(void) //wait for a child process to exit
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void) //grow process's memory by n bytes
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void) //sleep for n clock ticks
{
  int n;
  uint ticks0;

  argint(0, &n);
  if(n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  backtrace();
  return 0;
}

uint64
sys_kill(void) //terminate process [pid]
{
  int pid;

  argint(0, &pid);
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

uint64
sys_sigalarm(void)
{
  int interval;
  uint64 handler;
  argint(0, &interval);
  argaddr(1, &handler);
  myproc() -> interval = interval;
  myproc() -> handler = (void (*)()) handler;
  myproc() -> is_in_handler = 0;
  return 0;
}

uint64
sys_sigreturn(void)
{
  struct proc *proc_ptr = myproc();
  proc_ptr -> ticks = 0;
  proc_ptr -> trapframe -> epc = proc_ptr -> user_trapframe.epc;
  memmove((uint64 *) (proc_ptr->trapframe) + 5, (uint64 *) &(proc_ptr->user_trapframe) +1, 248); // dst, src, len
  proc_ptr -> is_in_handler = 0;
  yield();
  usertrapret();
  return 0;
}
