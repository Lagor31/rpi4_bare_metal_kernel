#include "include/SMP.h"

#include <stdint.h>

#include "include/Console.h"
#include "include/Core.h"
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
  Console::print("@@@@@@@@@@@@@@@\n\n Core %d active!\n\n@@@@@@@@@@@@@@@\n",
                 get_core());
  _wait_for_interrupt();

  /* // We're never getting here, our cores only respond to interrupts
  if (get_core() == 3) _hang_forever();
  while (true) {
    Core::spinms(100 + get_core() * 20);
    splck_lck(&sched_lock);
    Core::spinms(30);
    core_activations[get_core()]++;
    splck_done(&sched_lock);
    Console::print(
        "@@@@@@@@@@@@@@@@ Core %d still alive!"
        "@@@@@@@@@@@@@@@\n",
        get_core());
  } */
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
