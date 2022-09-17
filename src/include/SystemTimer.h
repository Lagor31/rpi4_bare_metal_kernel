/*
    Part of the Raspberry-Pi Bare Metal Tutorials
    https://www.valvers.com/rpi/bare-metal/
    Copyright (c) 2013-2018, Brian Sidebotham

    This software is licensed under the MIT License.
    Please see the LICENSE file included with this software.

*/

#ifndef RPI_SYSTIMER_H
#define RPI_SYSTIMER_H

#include <stdint.h>

#include "Lock.h"
#include "Mem.h"
#include "Driver.h"

#define RPI_SYSTIMER_BASE (MMIO::PERIPHERAL_BASE + 0x3000UL)

typedef struct {
  volatile uint32_t control_status;
  volatile uint32_t counter_lo;
  volatile uint32_t counter_hi;
  volatile uint32_t compare0;
  volatile uint32_t compare1;
  volatile uint32_t compare2;
  volatile uint32_t compare3;
} rpi_sys_timer_t;

class SystemTimer : public Driver {
 public:
  SystemTimer();
  void init();
  const char* getName();
  void unload();
  static void WaitMicroT1(uint32_t us);
  static void WaitMicroT3(uint32_t us);

  static rpi_sys_timer_t* getTimer();
};

#endif
