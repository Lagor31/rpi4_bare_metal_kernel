#ifndef KERNEL_HEAP_ALLOCATOR_H
#define KERNEL_HEAP_ALLOCATOR_H
#include "../Mem.h"

class KernelHeapAllocator : public MemoryAllocator {
 public:
  KernelHeapAllocator(){};
  KernelHeapAllocator(long *s, long *e) {
    if (created) return;
    ptr = (unsigned char *)s;
    bytes_left = (long)e - (long)s;
    created = true;
  };

  unsigned long freeSpace() { return bytes_left; }

  void *alloc(unsigned size) {
    if (created && size <= bytes_left) {
      bytes_left -= size;
      void *out = ptr;
      ptr += size;
      return out;
    }
    return (void *)0;
  };
  void free(void *p) { return; };
  void addChunk(long *, long *) { return; };

 public:
  unsigned char *ptr;
  long bytes_left;
  bool created;
};

static KernelHeapAllocator *main_kernel_alloc;

#endif