#include "include/Core.h"

#include "include/GIC.h"
#include "include/SystemTimer.h"

extern "C" void enable_irq(void);
extern "C" void disable_irq(void);

void Core::disableIRQ() { disable_irq(); }
void Core::enableIRQ() { enable_irq(); }

extern "C" void core_switch_to(Task *prev, Task *next);
Task *Core::current[4];
// Vector<Task *> *Core::runningQ[4];
Task *Core::runningQ[4][THREAD_N];
void Core::preemptDisable() { Core::current[get_core()]->c++; }
void Core::preemptEnable() { Core::current[get_core()]->c--; }
bool Core::isPreamptable() { return Core::current[get_core()]->c <= 0; }

void Core::switchTo(Task *next) {
  Task *current = Core::current[get_core()];
  if (current == next) {
    return;
  }

  Task *prev = current;
  Core::current[get_core()] = next;
  // Core::enableIRQ();
  core_switch_to(prev, next);
}

void Core::start(uint32_t core, void (*func)(void)) {
  if (core < 1 || core > 3) return;

  switch (core) {
    case 1:
      store64((unsigned long)0xffff0000000000E0, (unsigned long)func);
      asm volatile("dc civac, %0" : : "r"(0xffff0000000000E0) : "memory");
      asm volatile("sev");
      break;
    case 2:
      store64((unsigned long)0xffff0000000000E8, (unsigned long)func);
      asm volatile("dc civac, %0" : : "r"(0xffff0000000000E8) : "memory");
      asm volatile("sev");
      break;
    case 3:
      store64((unsigned long)0xffff0000000000F0, (unsigned long)func);
      asm volatile("dc civac, %0" : : "r"(0xffff0000000000F0) : "memory");
      asm volatile("sev");
      break;
  }
}

void Core::spinms(uint32_t n) {
  uint64_t counter = SystemTimer::getCounter();
  uint64_t target = (counter + (n * 1000));

  while (counter < target) {
    counter = SystemTimer::getCounter();
  }
}
