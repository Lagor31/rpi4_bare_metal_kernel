#include "Console.h"
#include "IRQ.h"
#include "Mem.h"
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

extern "C" void _wait_for_event();


extern "C" void kernel_main() {
  irq_init_vectors();
  enable_interrupt_controller();
  irq_enable();

  BootAllocator boot_allocator = BootAllocator(
      (unsigned char *)&_boot_alloc_start, (unsigned char *)&_boot_alloc_end);

  GlobalKernelAlloc::setAllocator(&boot_allocator);
  // We can use new with the boot allocator
  DriverManager::init();
  GPIO *gpio = new GPIO();
  UART *uart = new UART(gpio);
  Console::setKernelConsole(uart);

  timer_init();

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

  Vector<int> v = Vector<int>();
  v.push_back(31);
  v.push_back(33);
  v.push_back(32);
  v.push_back(35);

  Vector<int> v1 = v;
  Console::print("V:\n");
  for (int i : v) Console::print("%d\n", i);
  delete &v;
  Console::print("V1:\n");
  for (int i : v1) Console::print("%d\n", i);
  Console::print("3 elem: %d\n", v1[2]);

  String s = String("string1");
  String l = String(" && string 2");
  Console::print("S: %s\n", (s + l).get());
  s.swp(l);
  Console::print("Swap: %s\n", l.get());

  /* void *crash = (void *)0xffffffffffffffff;
  ((char *)crash)[0] = 'd';  
   */
  for (int i = 80; i > 0; --i) {
    wait_msec(2000);
    Console::print("#");
  }

  while (1) _wait_for_event();
}
