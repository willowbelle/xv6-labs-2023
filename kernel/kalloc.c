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
// pa to ref array idx
#define PA2IDX(pa) (((uint64)pa - (uint64)end) / PGSIZE)
#define PG_REFCNT(pa) (pageref.ref[(uint64)pa / PGSIZE])
struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct
{
  struct spinlock lock;
  int ref[PHYSTOP / PGSIZE];
} pageref;

void icrRef(void *pa)
{
  acquire(&pageref.lock);
  PG_REFCNT(pa)++;
  release(&pageref.lock);
}

void refcnt_init(void *pa)
{
  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("refcnt_init");
  acquire(&pageref.lock);
  PG_REFCNT(pa) = 1;
  release(&pageref.lock);
}

void dcrRef(void *pa)
{
  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("dcrRef");
  acquire(&pageref.lock);
  PG_REFCNT(pa)--;
  release(&pageref.lock);
}

int getRefCnt(void *pa)
{
  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("getRefCnt");
  acquire(&pageref.lock);
  int ret = PG_REFCNT(pa);
  release(&pageref.lock);
  return ret;
}

void kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&pageref.lock, "ref_cnt");
  memset(pageref.ref, 0, sizeof(pageref.ref));
  freerange(end, (void *)PHYSTOP);
}


void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void kfree(void *pa)
{
  struct run *r;

  if (((uint64)pa % PGSIZE) != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  if (getRefCnt(pa) > 1)
  {
    dcrRef(pa);
    return;
  }

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run *)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
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
  if(r) {
    kmem.freelist = r->next;
    refcnt_init((void*)r);
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
