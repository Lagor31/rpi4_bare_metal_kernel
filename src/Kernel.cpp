#include "ArmTimer.h"
#include "Console.h"
#include "GIC.h"
#include "IRQ.h"
#include "Mem.h"
#include "SMP.h"
#include "SystemTimer.h"
#include "drivers/Gpio.h"
#include "drivers/Uart.h"
#include "mem/BootAllocator.h"
#include "mem/KernelHeapAllocator.h"
#include "stdlib/Stdlib.h"
#include "stdlib/String.h"
#include "stdlib/Vector.h"

using namespace ltl::console;

extern void *_boot_alloc_start;
extern void *_boot_alloc_end;

extern void *_heap_start;
extern void *_heap_end;

extern "C" void wakeup_core(uint32_t core, uint64_t func);
extern "C" void _wait_for_event();

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
  Console::print("BootAlloc Start: 0x%x BootAlloc end: 0x%x\n",
                 &_boot_alloc_start, &_boot_alloc_end);
  Console::print("Heap Start: 0x%x Heap end: 0x%x\n",
                 (unsigned char *)&_heap_start, (unsigned char *)&_heap_end);

  KernelHeapAllocator *kha = new KernelHeapAllocator(
      (unsigned char *)&_heap_start, (unsigned char *)&_heap_end);
  GlobalKernelAlloc::setAllocator(kha);

  timerInit();
  /* void *crash = (void *)0xffffffffffffffff;
  ((char *)crash)[0] = 'd';
 */
  spin_msec(200);

  start_core3(&init_core);
  Console::print("Core 3 started");
  spin_msec(200);

  start_core2(&init_core);
  Console::print("Core 2 started");

  spin_msec(200);
  start_core1(&init_core);
  Console::print("Core 1 started");

  while (1) {
    /*  start_core2(print_core_id);
     start_core3(print_core_id); */

    _wait_for_event();
  }
}
