#include "../include/KernelHeapAllocator.h"

#include <stdint.h>

#include "../include/Console.h"

KernelHeapAllocator::KernelHeapAllocator(unsigned char *s, unsigned char *e) {
  ptr = s;
  bytes_left = (long)e - (long)s;
  l.l.val = SPLCK_UNLOCKED;
};

unsigned long KernelHeapAllocator::freeSpace() { return bytes_left; }

void *KernelHeapAllocator::alloc(unsigned size) {
  l.getLock();
  if (size <= bytes_left && bytes_left > 0) {
    void *out = ptr;
    ptr += size;
    bool isNotAligned = (uint64_t)ptr & 0x00000FFF;
    if (isNotAligned) {
      ptr = (unsigned char *)((uint64_t)ptr & 0xFFFFFFFFFFFFF000);
      ptr += 0x1000;
    }
    uint64_t effectiveSize = ((uint64_t)ptr - (uint64_t)out);
    bytes_left -= effectiveSize;
    l.release();
    Console::print("Alloc out=0x%x ptr=%x for size=%d ESize=0x%x\n", out, ptr,
                   size, effectiveSize);
    return out;
  }
  l.release();
  return (void *)0;
};
void KernelHeapAllocator::free(void *p) { return; };
void KernelHeapAllocator::addChunk(void *, void *) { return; };
