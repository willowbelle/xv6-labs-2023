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

// first address after kernel, defined by kernel.ld.
extern char end[]; 
                   
// free list element,every free page element stores in free page itself 
struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

// add a new superfreelist
struct {
  struct spinlock lock;
  struct run *freelist;
} superkmem;

/// @brief memory allocator init
void kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&superkmem.lock,"superkmem");
  freerange(end, (void*)PHYSTOP);
}

/// @brief call kfree, convert to char * type and align;xv6 assumes ram only 128MB
///        leave super pages for pass tests
/// @param pa_start 
/// @param pa_end 
void freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end - 10 * SUPERPGSIZE; p += PGSIZE) // here nums of super pages suggested by gpt
    kfree(p);
  p = (char *)SUPERPGROUNDUP((uint64)p);
  for(; p + SUPERPGSIZE <= (char*)pa_end; p += SUPERPGSIZE)
    superkfree(p);
}


/// @brief free a page pointed by pa;And add a free page to freelist(head insert)
/// @param pa physical addresss
void kfree(void *pa)
{
  struct run *r;
  // check
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs(有助于捕获悬挂引用).
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}


/// @brief just for super page use,basically same to kfree
/// @param pa 
void superkfree(void *pa){
  struct run *r;

  if((uint64)pa % SUPERPGSIZE != 0 || (char *)pa < end || (uint64)pa >= PHYSTOP)
    panic("superkfree");

  memset((void *)pa,1,SUPERPGSIZE);
  r = (struct run*)pa;

  acquire(&superkmem.lock);
  r->next = superkmem.freelist;
  superkmem.freelist = r;
  release(&superkmem.lock);
}


/// @brief allocate a page size from freelist, don't forger move freelist head
/// @param  none
/// @return return pointer if success,else return NULL
void * kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}


void * superkalloc(void){
  struct run *r;

  acquire(&superkmem.lock);
  r = superkmem.freelist;
  if(r)
    superkmem.freelist = r->next;
  release(&superkmem.lock);
  if(r)
    memset((char *)r,5,SUPERPGSIZE);
  return (void*)r;
}