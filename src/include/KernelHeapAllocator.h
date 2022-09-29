#ifndef KERNEL_HEAP_ALLOCATOR_H
#define KERNEL_HEAP_ALLOCATOR_H

#include "Mem.h"
#include "Spinlock.h"
class KernelHeapAllocator : public MemoryAllocator {
 public:
  KernelHeapAllocator(){};
  KernelHeapAllocator(unsigned char *s, unsigned char *e);
  unsigned long freeSpace();
  void *alloc(unsigned size);
  void free(void *p);
  void addChunk(void *, void *);
  unsigned char *getPtr() { return ptr; };

 private:
  unsigned char *ptr;
  uint64_t bytes_left;
  Spinlock l;
  /* You need space for the metadata and for the arena */
  struct buddy *buddy;
};

#endif