#include "include/SMP.h"

#include <stdint.h>

#include "include/Console.h"
#include "include/Core.h"
#include "include/GIC.h"
#include "include/IRQ.h"
#include "include/Stdlib.h"

extern "C" void _wait_for_interrupt();
extern "C" void c_init_core() {
  enable_irq();
  Console::print("@@@@@@@@@@@@@@@\n\n Core %d active!\n\n@@@@@@@@@@@@@@@\n",
                 get_core());
  _wait_for_interrupt();
}



