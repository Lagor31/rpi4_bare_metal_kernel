#include "include/BootAllocator.h"
#include "include/Console.h"
#include "include/GIC.h"
#include "include/Gpio.h"
#include "include/IRQ.h"
#include "include/KernelHeapAllocator.h"
#include "include/Lock.h"
#include "include/Mem.h"
#include "include/Mmu.h"
#include "include/SMP.h"
#include "include/Stdlib.h"
#include "include/String.h"
#include "include/SystemTimer.h"
#include "include/Uart.h"
#include "include/Vector.h"
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
  GIC400 *gic = new GIC400();
  SystemTimer *timer = new SystemTimer();
  DriverManager::load(gpio);
  DriverManager::load(uart);
  DriverManager::load(gic);
  DriverManager::load(timer);

  DriverManager::startAll();

  Console::setKernelConsole(uart);
  Console::print("\n\n\n\n\n\n############################################\n");
  Console::print("Current EL: %u\n", Std::getCurrentEL());

  Console::print("BootAlloc Start: 0x%x BootAlloc end: 0x%x\n",
                 &_boot_alloc_start, &_boot_alloc_end);
  Console::print("Heap Start: 0x%x Heap end: 0x%x\n",
                 (unsigned char *)&_heap_start, (unsigned char *)&_heap_end);

  KernelHeapAllocator *kha = new KernelHeapAllocator(
      (unsigned char *)&_heap_start, (unsigned char *)&_heap_end);
  GlobalKernelAlloc::setAllocator(kha);

  Console::print("List of loaded drivers:\n");
  for (auto d : *DriverManager::getAll())
    Console::print("Driver %s\n", d->getName());

  init_sched();

  start_core3(&init_core);
  spin_msec(10);
  start_core2(&init_core);
  spin_msec(10);
  start_core1(&init_core);
  spin_msec(50);

  timerInit();
  Console::print("############################################\n");

  _hang_forever();
}
