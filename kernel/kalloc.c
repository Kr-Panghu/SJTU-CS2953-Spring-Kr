// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem[NCPU];

void
kfreesingle(void *pa, int cpuid, int flag)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catc=h dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem[cpuid].lock);
  r->next = kmem[cpuid].freelist;
  kmem[cpuid].freelist = r;
  release(&kmem[cpuid].lock);
}

void
kinit()
{
  for(int i = 0; i < NCPU; ++i){
    initlock(&kmem[i].lock, "kmem");
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  int ind = 0;
  p = (char*)PGROUNDUP((uint64)pa_start);

  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    kfreesingle(p, ind, 0);
    ind = (ind + 1) % NCPU;
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  // struct run *r;

  // if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
  //   panic("kfree");

  // // Fill with junk to catch dangling refs.
  // memset(pa, 1, PGSIZE);

  // r = (struct run*)pa;

  // acquire(&kmem.lock);
  // r->next = kmem.freelist;
  // kmem.freelist = r;
  // release(&kmem.lock);

  // it's only safe to call cpuid() and use its result when interrupts are turned off
  push_off();
  kfreesingle(pa, cpuid(), 1);
  pop_off();
}

void *
steal_freelist(void) {
  struct run *r = 0;
  push_off();
  int id_now = cpuid();
  int id_next = id_now;
  release(&kmem[id_now].lock);
  while (1) {
    id_next = (id_next + 1) % NCPU;
    if (id_next == id_now) // The free list of any CPU is empty
      break;
    acquire(&kmem[id_next].lock);
    r = kmem[id_next].freelist;
    if(r) {
      kmem[id_next].freelist = r->next;
      release(&kmem[id_next].lock);
      break;
    }
    release(&kmem[id_next].lock);
  }
  acquire(&kmem[id_now].lock);
  pop_off();
  return (void*)r;
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  push_off();
  int id = cpuid();

  acquire(&kmem[id].lock);
  r = kmem[id].freelist;
  if(r)
    kmem[id].freelist = r->next;
  else
    r = steal_freelist();

  release(&kmem[id].lock);
  pop_off();

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
