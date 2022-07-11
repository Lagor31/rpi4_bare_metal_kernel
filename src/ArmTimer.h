#ifndef ARM_TIMER_H
#define ARM_TIMER_H

#include "Console.h"
#include "IRQ.h"
#include "Mem.h"
#include <stdint.h>


#define CLOCKHZ 1000000

using namespace ltl::console;

const unsigned int timer1_int = CLOCKHZ;
const unsigned int timer3_int = CLOCKHZ / 4;

struct timer_regs {
  volatile unsigned int control_status;
  volatile unsigned int counter_lo;
  volatile unsigned int counter_hi;
  volatile unsigned int compare[4];
};

#define REGS_TIMER ((struct timer_regs*)(MMIO::PERIPHERAL_BASE + 0x00003000))


#define CORE0_TIMER_IRQCNTL 0x40000040
#define CORE0_IRQ_SOURCE 0x40000060

extern unsigned int timer1_val;
extern unsigned int timer3_val;
extern uint32_t cntfrq;
void armTimerInit();
void spin_msec(unsigned int n);
#endif