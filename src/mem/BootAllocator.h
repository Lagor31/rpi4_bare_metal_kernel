#ifndef BOOT_ALLOCATOR_H
#define BOOT_ALLOCATOR_H
#include "../Mem.h"

class BootAllocator : public MemoryAllocator {
 public:
  BootAllocator(){};
  BootAllocator(long *s, long *e) {
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

 private:
  unsigned char *ptr;
  long bytes_left;
  bool created;
};

#endif