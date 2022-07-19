#ifndef MEM_H
#define MEM_H

#include <stddef.h>

class MMIO {
 public:
  static void write(long reg, unsigned int val);
  static unsigned int read(long reg);

  enum ADDR {
    PERIPHERAL_BASE = 0xFE000000,
    GPFSEL0 = PERIPHERAL_BASE + 0x200000,
    GPSET0 = PERIPHERAL_BASE + 0x20001C,
    GPCLR0 = PERIPHERAL_BASE + 0x200028,
    GPPUPPDN0 = PERIPHERAL_BASE + 0x2000E4
  };
};

class MemoryAllocator {
 public:
  MemoryAllocator(){};
  MemoryAllocator(void *, void *);
  virtual void *alloc(unsigned size) = 0;
  virtual void free(void *p) = 0;
  virtual void addChunk(void *, void *) = 0;
  virtual unsigned long freeSpace() = 0;
  virtual unsigned char *getPtr() = 0;
};

/*
Global Kernel Allocator
Can be set to use different backends at runtime
*/

class GlobalKernelAlloc {
 private:
  static MemoryAllocator *kalloc;

 public:
  static MemoryAllocator *getAllocator();
  static void setAllocator(MemoryAllocator *in);
  static void *alloc(unsigned size);
  static void free(void *p);
  static unsigned long freeSpace();
};

#endif