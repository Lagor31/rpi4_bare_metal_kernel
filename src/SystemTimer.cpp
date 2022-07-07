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

void RPI_WaitMicroSeconds(uint32_t us) {
  volatile uint32_t ts = rpiSystemTimer->counter_lo;
  rpiSystemTimer->compare1 = ts + us;
  //rpiSystemTimer->compare3 = ts + us * 2;

  /* while ((rpiSystemTimer->counter_lo - ts) < us) {
  } */
}
