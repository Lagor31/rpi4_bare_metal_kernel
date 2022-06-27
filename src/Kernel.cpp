#include "Console.h"
#include "drivers/Gpio.h"
#include "drivers/Uart.h"
#include "Mem.h"
#include "mem/BootAllocator.h"
#include "mem/KernelHeapAllocator.h"
extern void *_boot_alloc_start;
extern void *_boot_alloc_end;

extern void *_heap_start;
extern void *_heap_end;

extern "C" void kernel_main() {
  BootAllocator boot_allocator =
      BootAllocator((long *)&_boot_alloc_start, (long *)&_boot_alloc_end);

  KernelAlloc::setAllocator(&boot_allocator);
  // We can use new with the boot allocator

  main_kernel_alloc =
      new KernelHeapAllocator((long *)&_heap_start, (long *)&_heap_end);

  KernelAlloc::setAllocator(main_kernel_alloc);

  DriverManager::init();
  GPIO *gpio = new GPIO();
  UART *uart = new UART(gpio);
  Console::setKernelConsole(uart);
  Console::print("GPIO Addr: 0x%x\n", gpio);
  Console::print("UART Addr: 0x%x\n", uart);
  DriverManager::load(gpio);
  DriverManager::load(uart);

  main_kernel_alloc =
      new KernelHeapAllocator((long *)&_heap_start, (long *)&_heap_end);

  //KernelAlloc::setAllocator(main_kernel_alloc);

  Console::print("Obj1 Addr: 0x%x\n", new GPIO());
  Console::print("Obj2 Addr: 0x%x\n", new GPIO());
  Console::print("Obj3 Addr: 0x%x\n", new GPIO());
  Console::print("Obj4 Addr: 0x%x\n", new GPIO());
  Console::print("Obj5 Addr: 0x%x\n", new GPIO());

  while (1)
    ;
}
