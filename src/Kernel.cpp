#include "Console.h"
#include "Mem.h"
#include "drivers/Gpio.h"
#include "drivers/Uart.h"
#include "mem/BootAllocator.h"
#include "mem/KernelHeapAllocator.h"

extern void *_boot_alloc_start;
extern void *_boot_alloc_end;

extern void *_heap_start;
extern void *_heap_end;
extern void *_start;

extern "C" void kernel_main() {
  BootAllocator boot_allocator = BootAllocator(
      (unsigned char *)&_boot_alloc_start, (unsigned char *)&_boot_alloc_end);

  GlobalKernelAlloc::setAllocator(&boot_allocator);
  // We can use new with the boot allocator
  MemoryAllocator *t = GlobalKernelAlloc::getAllocator();
  DriverManager::init();
  GPIO *gpio = new GPIO();
  UART *uart = new UART(gpio);
  Console::setKernelConsole(uart);

  Console::print("GAlloc 0x%x\n", GlobalKernelAlloc::getAllocator());

  Console::print("Stack top: 0x%x\n", &_start);
  Console::print("BootAlloc Addr: 0x%x\n", &boot_allocator);

  Console::print("GPIO Addr: 0x%x\n", gpio);
  Console::print("UART Addr: 0x%x\n", uart);
  DriverManager::load(gpio);
  DriverManager::load(uart);

  Console::print("Obj1 Addr: 0x%x\n", new GPIO());
  Console::print("Obj2 Addr: 0x%x\n", new GPIO());
  Console::print("Obj3 Addr: 0x%x\n", new GPIO());
  Console::print("BootAlloc Start: 0x%x BootAlloc end: 0x%x\n",
                 &_boot_alloc_start, &_boot_alloc_end);

  Console::print("Heap Start: 0x%x Heap end: 0x%x\n",  (unsigned char *)&_heap_start,  (unsigned char *)&_heap_end);

  KernelHeapAllocator *kha = new KernelHeapAllocator(
      (unsigned char *)&_heap_start, (unsigned char *)&_heap_end);

  Console::print("KHA Addr: 0x%x\n", kha);
  Console::print("KHA ptr: 0x%x\n", kha->getPtr());
  Console::print("char * size: %d\n", sizeof(char *));
  GlobalKernelAlloc::setAllocator(kha);
  Console::print("Set KHA Allocator\n");
  Console::print("GAlloc 0x%x\n", GlobalKernelAlloc::getAllocator());
  Console::print("KHA ptr: 0x%x\n",
                 GlobalKernelAlloc::getAllocator()->getPtr());
  Console::print("Obj4 Addr: 0x%x\n", new GPIO());
  Console::print("Obj5 Addr: 0x%x\n", new GPIO());

  while (1)
    ;
}
