#include "include/Core.h"
#include "include/SystemTimer.h"

extern "C" void enable_irq(void);
extern "C" void disable_irq(void);

void Core::disableIRQ() { disable_irq(); }
void Core::enableIRQ() { enable_irq(); }

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

void Core::spinms(uint64_t n) {
  rpi_sys_timer_t *sys_timer = SystemTimer::getTimer();

  unsigned int target = sys_timer->counter_lo + (n * 1000);
  unsigned int t = sys_timer->counter_lo;

  while (t < target) {
    t = sys_timer->counter_lo;
  }
}
