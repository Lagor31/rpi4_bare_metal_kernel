/*
    Part of the Raspberry-Pi Bare Metal Tutorials
    https://www.valvers.com/rpi/bare-metal/
    Copyright (c) 2013-2018, Brian Sidebotham

    This software is licensed under the MIT License.
    Please see the LICENSE file included with this software.

*/

#include "SystemTimer.h"

#include <stdint.h>

static rpi_sys_timer_t* rpiSystemTimer = (rpi_sys_timer_t*)RPI_SYSTIMER_BASE;

rpi_sys_timer_t* RPI_GetSystemTimer(void) { return rpiSystemTimer; }

void RPI_WaitMicroSecondsT1(uint32_t us) {
  rpiSystemTimer->compare1 = rpiSystemTimer->counter_lo + us;
}

void RPI_WaitMicroSecondsT3(uint32_t us) {
  rpiSystemTimer->compare3 = rpiSystemTimer->counter_lo + us;
}
