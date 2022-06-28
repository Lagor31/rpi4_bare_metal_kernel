#include "Console.h"
#include "Mem.h"
#include "drivers/Gpio.h"
#include "drivers/Uart.h"
#include "mem/BootAllocator.h"
#include "mem/KernelHeapAllocator.h"
#include "stdlib/Stdlib.h"

extern void *_boot_alloc_start;
extern void *_boot_alloc_end;

extern void *_heap_start;
extern void *_heap_end;

extern "C" void _wait_for_event();

extern "C" void kernel_main() {
  BootAllocator boot_allocator = BootAllocator(
      (unsigned char *)&_boot_alloc_start, (unsigned char *)&_boot_alloc_end);

  GlobalKernelAlloc::setAllocator(&boot_allocator);
  // We can use new with the boot allocator
  DriverManager::init();
  GPIO *gpio = new GPIO();
  UART *uart = new UART(gpio);
  Console::setKernelConsole(uart);
  DriverManager::load(gpio);
  DriverManager::load(uart);
  Console::print("BootAlloc Start: 0x%x BootAlloc end: 0x%x\n",
                 &_boot_alloc_start, &_boot_alloc_end);
  Console::print("Heap Start: 0x%x Heap end: 0x%x\n",
                 (unsigned char *)&_heap_start, (unsigned char *)&_heap_end);

  KernelHeapAllocator *kha = new KernelHeapAllocator(
      (unsigned char *)&_heap_start, (unsigned char *)&_heap_end);
  GlobalKernelAlloc::setAllocator(kha);

  Vector<int> v = Vector<int>();
  v.push_back(31);
  v.push_back(33);
  v.push_back(32);
  v.push_back(35);

  // v->push_back(1);
  for (int i : v) Console::print("%d\n", i);

  while (1) _wait_for_event();
}
