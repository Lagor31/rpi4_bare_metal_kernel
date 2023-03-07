/*
    Part of the Raspberry-Pi Bare Metal Tutorials
    https://www.valvers.com/rpi/bare-metal/
    Copyright (c) 2013-2018, Brian Sidebotham

    This software is licensed under the MIT License.
    Please see the LICENSE file included with this software.

*/

#include "../include/SystemTimer.h"

#include <stdint.h>

#include "../include/Core.h"
#include "../include/Spinlock.h"

static rpi_sys_timer_t* rpiSystemTimer = (rpi_sys_timer_t*)RPI_SYSTIMER_BASE;

rpi_sys_timer_t* SystemTimer::getTimer(void) {
  // lock->getLock();
  rpi_sys_timer_t* out = rpiSystemTimer;
  // lock->release();
  return out;
}

Spinlock* SystemTimer::lock;

SystemTimer::SystemTimer() {}

void SystemTimer::WaitMicroT1(uint32_t us) {
  uint32_t lo;
  uint32_t hi;
  do {
    lo = rpiSystemTimer->counter_lo;
    hi = rpiSystemTimer->counter_hi;
  } while (hi != rpiSystemTimer->counter_hi);

  rpiSystemTimer->compare1 = (lo + us);
}

void SystemTimer::WaitMicroT3(uint32_t us) {
  uint32_t lo;
  uint32_t hi;
  do {
    lo = rpiSystemTimer->counter_lo;
    hi = rpiSystemTimer->counter_hi;
  } while (hi != rpiSystemTimer->counter_hi);
  rpiSystemTimer->compare3 = (lo + us);
}

uint64_t SystemTimer::getCounter() {
  uint32_t lo;
  uint32_t hi;
  do {
    lo = rpiSystemTimer->counter_lo;
    hi = rpiSystemTimer->counter_hi;
  } while (hi != rpiSystemTimer->counter_hi);

  uint64_t out = hi;
  out <<= (static_cast<uint64_t>(32));
  out += lo;
  return out;
}

void SystemTimer::init() { lock = new Spinlock(); }
const char* SystemTimer::getName() { return "SystemTimer"; }
void SystemTimer::unload() {}