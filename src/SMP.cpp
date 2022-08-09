#include "SMP.h"

#include <stdint.h>

#include "Console.h"
#include "GIC.h"
#include "IRQ.h"
#include "stdlib/Stdlib.h"

using ltl::console::Console;
uint32_t first = 0;

extern "C" void c_init_core() {
  enable_irq();
  Console::print("@@@@@@@@@@@@@@@@\n\n Core %d active!\n\n@@@@@@@@@@@@@@@\n",
                 get_core());
  _hang_forever();

  while (true) {
    spin_msec(get_core() * 50);

    /*  if (get_core() == 2 && first == 0) {
       uint64_t *err = (uint64_t *)0xffffffffffffff0;
       *err = 1234567;

       Console::print("\n\n\nRecovered from exception \n\n");
       first++;
     } */

    Console::print(
        "@@@@@@@@@@@@@@@@ Core %d still alive at EL=%d! @@@@@@@@@@@@@@@\n",
        get_core(), get_el());

    asm volatile("wfe");
  }
}

void start_core1(void (*func)(void)) {
  store64((unsigned long)0xffff000000000000 + 0xE0, (unsigned long)func);
  asm volatile("dc civac, %0" : : "r"(0xE0) : "memory");
  asm volatile("sev");
}

void start_core2(void (*func)(void)) {
  store64((unsigned long)0xffff000000000000 + 0xE8, (unsigned long)func);
  asm volatile("dc civac, %0" : : "r"(0xE8) : "memory");
  asm volatile("sev");
}

void start_core3(void (*func)(void)) {
  store64((unsigned long)0xffff000000000000 + 0xF0, (unsigned long)func);
  asm volatile("dc civac, %0" : : "r"(0xF0) : "memory");
  asm volatile("sev");
}

void store64(unsigned long address, unsigned long value) {
  *(unsigned long *)address = value;
}

unsigned long load64(unsigned long address) {
  return *(unsigned long *)address;
}

void store32(unsigned long address, unsigned int value) {
  *(unsigned int *)address = value;
}

unsigned int load32(unsigned long address) { return *(unsigned int *)address; }

void print_core_id() { Console::print("Running on core: %d\n", get_core()); }
