#include "SMP.h"

#include <stdint.h>

#include "Console.h"
#include "GIC.h"
#include "stdlib/Stdlib.h"

using namespace ltl::console;

extern "C" void wakeup_core(unsigned int core, unsigned long func);

void store32(unsigned long address, unsigned long value) {
  *(unsigned long *)address = value;
}

extern "C" void c_init_core() {
  Console::print("@@@@@@@@@@@@@@@@\n\n Core %d active! \n\n@@@@@@@@@@@@@@@\n",
                 get_core());
  while (true) {
    Console::print("@@@@@@@@@@@@@@@@ Core %d still alive! @@@@@@@@@@@@@@@\n",
                   get_core());
    spin_msec(200);
    //_wait_for_event();
  }
}
void start_core1(void (*func)(void)) {
  store32((unsigned long)0xE0, (unsigned long)func);
  asm volatile("dc civac, %0" : : "r"(0xE0) : "memory");
  asm volatile("sev");
}

void start_core2(void (*func)(void)) {
  store32((unsigned long)0xE8, (unsigned long)func);
  asm volatile("dc civac, %0" : : "r"(0xE8) : "memory");
  asm volatile("sev");
}

void start_core3(void (*func)(void)) {
  store32((unsigned long)0xF0, (unsigned long)func);
  asm volatile("dc civac, %0" : : "r"(0xF0) : "memory");
  asm volatile("sev");
}

void store64(unsigned long address, unsigned long long value) {
  *(unsigned long *)address = value;
}

unsigned long load32(unsigned long address) {
  return *(unsigned long *)address;
}
unsigned long load64(unsigned long address) {
  return *(unsigned long *)address;
}
unsigned int *_core_1_wakeup = (unsigned int *)0xe4;
unsigned int *_core_1_wakeup0 = (unsigned int *)0xe0;

#define CORE1_ADDR ((volatile __attribute__((aligned(4))) uint64_t *)(0xE0))

void print_core_id() {
  Console::print("Running on core: %d\n", get_core());
  // while (true) _wait_for_event();
}
