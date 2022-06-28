#include "KernelHeapAllocator.h"

#include "../Console.h"

KernelHeapAllocator::KernelHeapAllocator(unsigned char *s, unsigned char *e) {
  ptr = s;
  Console::print("In constructor\ns = 0x%x ptr = 0x%x\n", s, ptr);
  bytes_left = (long)e - (long)s;
};

unsigned long KernelHeapAllocator::freeSpace() { return bytes_left; }

void *KernelHeapAllocator::alloc(unsigned size) {
  if (size <= bytes_left) {
    bytes_left -= size;
    void *out = ptr;
    ptr += size;
    return out;
  }
  return (void *)0;
};
void KernelHeapAllocator::free(void *p) { return; };
void KernelHeapAllocator::addChunk(void *, void *) { return; };
