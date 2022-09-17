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
}



void print_core_id() { Console::print("Running on core: %d\n", get_core()); }
