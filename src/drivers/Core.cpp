#include "../include/Core.h"

#include "../include/SystemTimer.h"

void Core::spinms(uint64_t n) {
  rpi_sys_timer_t* sys_timer = SystemTimer::getTimer();

  unsigned int target = sys_timer->counter_lo + (n * 1000);
  unsigned int t = sys_timer->counter_lo;

  while (t < target) {
    t = sys_timer->counter_lo;
  }
}
