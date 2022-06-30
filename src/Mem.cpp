#include "Mem.h"

#include "Console.h"
#include "mem/KernelHeapAllocator.h"

MemoryAllocator *GlobalKernelAlloc::kalloc = nullptr;

MemoryAllocator *GlobalKernelAlloc::getAllocator() {
  return GlobalKernelAlloc::kalloc;
};

void GlobalKernelAlloc::setAllocator(MemoryAllocator *in) {
  GlobalKernelAlloc::kalloc = in;
}

void *GlobalKernelAlloc::alloc(unsigned size) {
  return GlobalKernelAlloc::getAllocator()->alloc(size);
}
void GlobalKernelAlloc::free(void *p) {
  return GlobalKernelAlloc::getAllocator()->free(p);
};
unsigned long GlobalKernelAlloc::freeSpace() {
  return getAllocator()->freeSpace();
}

void MMIO::write(long reg, unsigned int val) {
  *(volatile unsigned int *)reg = val;
}

unsigned int MMIO::read(long reg) { return *(volatile unsigned int *)reg; }

void *operator new(size_t size) {
  Console::print("New called\n");
  return GlobalKernelAlloc::alloc(size);
}
void *operator new[](size_t size) {
  Console::print("New[] called\n");
  return GlobalKernelAlloc::alloc(size);
}
void operator delete(void *p) {
  Console::print("Delete called\n");
  GlobalKernelAlloc::free(p);
}
void operator delete(void *p, unsigned long) {
  Console::print("Delete long called\n");
  GlobalKernelAlloc::free(p);
}
void operator delete[](void *p) {
  Console::print("Delete[] called\n");
  GlobalKernelAlloc::free(p);
}

// Keep compiler happy about pure virtual methods
extern "C" void __cxa_pure_virtual() {}
