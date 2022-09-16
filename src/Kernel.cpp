#include "Console.h"
#include "GIC.h"
#include "IRQ.h"
#include "Lock.h"
#include "Mem.h"
#include "SMP.h"
#include "SystemTimer.h"
#include "drivers/Gpio.h"
#include "drivers/Uart.h"
#include "mem/BootAllocator.h"
#include "mem/KernelHeapAllocator.h"
#include "mmu/Mmu.h"
#include "stdlib/Stdlib.h"
#include "stdlib/String.h"
#include "stdlib/Vector.h"
using ltl::console::Console;

extern void *_boot_alloc_start;
extern void *_boot_alloc_end;

extern void *_heap_start;
extern void *_heap_end;

extern "C" void wakeup_core(uint32_t core, uint64_t func);

extern "C" void init_core();
extern "C" void kernel_main() {
  BootAllocator boot_allocator = BootAllocator(
      (unsigned char *)&_boot_alloc_start, (unsigned char *)&_boot_alloc_end);

  GlobalKernelAlloc::setAllocator(&boot_allocator);
  // We can use new with the boot allocator
  DriverManager::init();
  GPIO *gpio = new GPIO();
  UART *uart = new UART(gpio);
  Console::setKernelConsole(uart);
  Console::print("\n\n\n\n\n\n######################\n");
  Console::print("Current EL: %u\n", Std::getCurrentEL());
  DriverManager::load(gpio);
  DriverManager::load(uart);
  /*   Console::print("BootAlloc Start: 0x%x BootAlloc end: 0x%x\n",
                   &_boot_alloc_start, &_boot_alloc_end);
    Console::print("Heap Start: 0x%x Heap end: 0x%x\n",
                   (unsigned char *)&_heap_start, (unsigned char *)&_heap_end);
  */

  KernelHeapAllocator *kha = new KernelHeapAllocator(
      (unsigned char *)&_heap_start, (unsigned char *)&_heap_end);
  GlobalKernelAlloc::setAllocator(kha);

  init_sched();
  
  start_core3(&init_core);
  spin_msec(10);
  start_core2(&init_core);
  spin_msec(10);
  start_core1(&init_core);
  spin_msec(50);

  timerInit();

  _hang_forever();
}
