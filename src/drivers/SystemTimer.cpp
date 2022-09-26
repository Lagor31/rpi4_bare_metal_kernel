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
  uint32_t lo = rpiSystemTimer->counter_lo;
  rpiSystemTimer->compare1 = (lo + us) % 0xFFFFFFFF;
}

void SystemTimer::WaitMicroT3(uint32_t us) {
  rpiSystemTimer->compare3 = (rpiSystemTimer->counter_lo + us) % 0xFFFFFFFF;
}

uint64_t SystemTimer::getCounter() {
  // Core::preemptDisable();
  //  lock->getLock();
  uint32_t lo = rpiSystemTimer->counter_lo;
  uint32_t hi = rpiSystemTimer->counter_hi;
  if (hi != rpiSystemTimer->counter_hi) {
    hi = rpiSystemTimer->counter_hi;
    lo = rpiSystemTimer->counter_lo;
  }
  // lock->release();
  // Core::preemptEnable();
  uint64_t out = (hi << (static_cast<uint64_t>(16)));
  out <<= 16;
  out += lo;
  return out;
}

void SystemTimer::init() { lock = new Spinlock(); }
const char* SystemTimer::getName() { return "SystemTimer"; }
void SystemTimer::unload() {}