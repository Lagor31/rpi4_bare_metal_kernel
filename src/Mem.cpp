#include "include/Mem.h"

#include "include/Console.h"
#include "include/KernelHeapAllocator.h"
#include "include/SMP.h"
#include "include/Spinlock.h"

void store64(unsigned long address, unsigned long value) {
  *(unsigned long *)address = (unsigned long)value;
}

unsigned long load64(unsigned long address) {
  return *(unsigned long *)address;
}

void store32(unsigned long address, unsigned int value) {
  *(unsigned int *)address = value;
}

unsigned int load32(unsigned long address) { return *(unsigned int *)address; }

MemoryAllocator *GlobalKernelAlloc::kalloc = nullptr;

MemoryAllocator *GlobalKernelAlloc::getAllocator() {
  return GlobalKernelAlloc::kalloc;
};

void GlobalKernelAlloc::setAllocator(MemoryAllocator *in) {
  GlobalKernelAlloc::kalloc = in;
}

void *GlobalKernelAlloc::alloc(unsigned size) {
  // Console::print("Alloc %d bytes\n", size);
  return GlobalKernelAlloc::getAllocator()->alloc(size);
}
void GlobalKernelAlloc::free(void *p) {
  // Console::print("Free %x ptr\n", p);
  return GlobalKernelAlloc::getAllocator()->free(p);
};
unsigned long GlobalKernelAlloc::freeSpace() {
  return getAllocator()->freeSpace();
}

void MMIO::write(long reg, unsigned int val) { *(unsigned int *)reg = val; }

unsigned int MMIO::read(unsigned long reg) {
  unsigned int out = *(unsigned int *)reg;
  return out;
}

void *operator new(size_t size) {
  // Console::print_no_lock("New called Size: %d\n", size);
  return GlobalKernelAlloc::alloc(size);
}
void *operator new[](size_t size) {
  // Console::print_no_lock("New[] called Size: %d\n", size);
  return GlobalKernelAlloc::alloc(size);
}
void operator delete(void *p) {
  // Console::print_no_lock("Delete called\n");
  GlobalKernelAlloc::free(p);
}
void operator delete(void *p, unsigned long s) {
  // Console::print_no_lock("Delete long called Size: %d\n", s);
  GlobalKernelAlloc::free(p);
}
void operator delete[](void *p) {
  // Console::print_no_lock("Delete[] called\n");
  GlobalKernelAlloc::free(p);
}

// Keep compiler happy about pure virtual methods
extern "C" void __cxa_pure_virtual() {}
