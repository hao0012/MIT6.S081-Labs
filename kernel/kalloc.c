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

#define PA2IDX(p) (((p) - KERNBASE) / PGSIZE)

struct spinlock mylock;
int pageref[(PHYSTOP - KERNBASE) / PGSIZE];

#define GET(p) pageref[PA2IDX((uint64)(p))]

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&mylock, "pgref");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&mylock);
  if (--GET(pa) <= 0) {

    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
  release(&mylock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    GET(r) = 1;
  }
  return (void*)r;
}

void *copy_page(void *pa) {
  acquire(&mylock);

  if (GET(pa) <= 1) {
    release(&mylock);
    return pa;
  }

  uint64 newpa = (uint64)kalloc();
  if (newpa == 0) {
    release(&mylock);
    return 0;
  }

  memmove((void *)newpa, (void *)pa, PGSIZE);

  GET(pa)--;

  release(&mylock);
  return (void *)newpa;
}

void incr(void *pa) {
  acquire(&mylock);
  GET(pa)++;
  release(&mylock);
}