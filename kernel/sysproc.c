#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
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
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
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
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
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
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.  
  uint64 base;
  uint64 bitmask;  
  int len;
 
  pagetable_t pagetable = 0;
  unsigned int procmask = 0 ;
  pte_t *pte;
 
  struct proc *p = myproc();
 
  argaddr(0, &base);      // Starting virtual address of the first user page to check
  argint(1, &len);        // The number of pages to check
  argaddr(2, &bitmask);   // Take a user address to a buffer to store the results into a bitmask

  // Upper limit on the number of pages that can be scanned
  if (len > sizeof(int)*8) len = sizeof(int)*8;
 
  for(int i=0; i<len; i++) {
    pagetable = p->pagetable;
      
    if(base >= MAXVA)
      panic("pgaccess");
 
    for(int level = 2; level > 0; level--) {
      pte = &pagetable[PX(level, base)];
      if(*pte & PTE_V) {
        pagetable = (pagetable_t)PTE2PA(*pte);
      } else {
        return -1;
      }      
    }
    pte = &pagetable[PX(0, base)];
    if(pte == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0)  // unvalid or not user-accessible
      return -1; 
    if(*pte & PTE_A) {                                          // have been accessed
      procmask = procmask | (1L << i);
      *pte = *pte & (~PTE_A);
    }
    base += PGSIZE;
  }
 
  pagetable = p->pagetable;
  return copyout(pagetable, bitmask, (char *) &procmask, sizeof(unsigned int)); // error: return -1
}
#endif

uint64
sys_kill(void)
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
