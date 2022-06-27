#include "Mem.h"

MemoryAllocator *KernelAlloc::kalloc;

void KernelAlloc::setAllocator(MemoryAllocator *in) {
  KernelAlloc::kalloc = in;
}
void *KernelAlloc::alloc(unsigned size) {
  return KernelAlloc::kalloc->alloc(size);
}
void KernelAlloc::free(void *p) { return KernelAlloc::kalloc->free(p); };
unsigned long KernelAlloc::freeSpace() { return kalloc->freeSpace(); }

void MMIO::write(long reg, unsigned int val) {
  *(volatile unsigned int *)reg = val;
}

unsigned int MMIO::read(long reg) { return *(volatile unsigned int *)reg; }

void *operator new(size_t size) { return KernelAlloc::alloc(size); }
void *operator new[](size_t size) { return KernelAlloc::alloc(size); }

void operator delete(void *p) { KernelAlloc::free(p); }
void operator delete(void *p, unsigned long) { KernelAlloc::free(p); }

void operator delete[](void *p) { KernelAlloc::free(p); }

// Keep compiler happy about pure virtual methods
extern "C" void __cxa_pure_virtual() {}
