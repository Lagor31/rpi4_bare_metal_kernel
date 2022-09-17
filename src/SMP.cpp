#include "include/SMP.h"

#include <stdint.h>

#include "include/Console.h"
#include "include/GIC.h"
#include "include/IRQ.h"
#include "include/Stdlib.h"

using ltl::console::Console;
uint32_t first = 0;
extern uint64_t core_activations[4];
extern splck_t sched_lock;
extern "C" void _wait_for_interrupt();

extern "C" void c_init_core() {
  //_hang_forever();
  enable_irq();
  // spin_msec(1000);
  Console::print("@@@@@@@@@@@@@@@\n\n Core %d active!\n\n@@@@@@@@@@@@@@@\n",
                 get_core());
  _wait_for_interrupt();
  //_hang_forever();

  if (get_core() == 3) _hang_forever();
  // spin_msec(2000);
  while (true) {
    // asm volatile("wfe");

    /*  if (get_core() == 2 && first == 0) {
       uint64_t *err = (uint64_t *)0xffffffffffffff0;
       *err = 1234567;

       Console::print("\n\n\nRecovered from exception \n\n");
       first++;
     } */

    spin_msec(100 + get_core() * 20);

    splck_lck(&sched_lock);
    // if(get_core() == 3)
    spin_msec(30);

    core_activations[get_core()]++;
    // Console::print("Core%d alive!\n", get_core());

    splck_done(&sched_lock);

    Console::print(
        "@@@@@@@@@@@@@@@@ Core %d still alive!"
        "@@@@@@@@@@@@@@@\n",
        get_core());
    /* Console::print_special(
        "@@@@@@@@@@@@@@@@ Core %d still alive!"
        "@@@@@@@@@@@@@@@\n",
        get_core()); */
    /*
        if ((core_activations[get_core()] % 50 == 0))
          Console::print("@@@@@@@@@@@@@@@@ Core %d still alive!
       @@@@@@@@@@@@@@@\n", get_core()); */
  }
}

void start_core1(void (*func)(void)) {
  store64((unsigned long)0xffff0000000000E0, (unsigned long)func);
  asm volatile("dc civac, %0" : : "r"(0xffff0000000000E0) : "memory");
  asm volatile("sev");
}

void start_core2(void (*func)(void)) {
  store64((unsigned long)0xffff0000000000E8, (unsigned long)func);
  asm volatile("dc civac, %0" : : "r"(0xffff0000000000E8) : "memory");
  asm volatile("sev");
}

void start_core3(void (*func)(void)) {
  store64((unsigned long)0xffff0000000000F0, (unsigned long)func);
  asm volatile("dc civac, %0" : : "r"(0xffff0000000000F0) : "memory");
  asm volatile("sev");
}

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

void print_core_id() { Console::print("Running on core: %d\n", get_core()); }
