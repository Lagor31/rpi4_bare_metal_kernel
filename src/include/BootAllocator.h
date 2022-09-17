#ifndef BOOT_ALLOCATOR_H
#define BOOT_ALLOCATOR_H
#include "Mem.h"

class BootAllocator : public MemoryAllocator {
 public:
  BootAllocator(){};
  BootAllocator(unsigned char *s, unsigned char *e) {
    ptr = s;
    bytes_left = (long)e - (long)s;
  };

  unsigned long freeSpace() { return bytes_left; }

  void *alloc(unsigned size) {
    if (size <= bytes_left) {
      bytes_left -= size;
      unsigned char *out = ptr;
      ptr += size;
      return out;
    }
    return (void *)0;
  };

  void free(void *p) { return; };
  void addChunk(void *, void *) { return; };
  unsigned char *getPtr() { return ptr; };

 private:
  unsigned char *ptr;
  long bytes_left;
};

#endif