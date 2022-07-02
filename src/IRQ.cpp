#include "Console.h"
#include "Mem.h"
#include "boot/sysregs.h"
#include "stdlib/Stdlib.h"

using namespace ltl::console;

struct timer_regs {
  volatile unsigned int control_status;
  volatile unsigned int counter_lo;
  volatile unsigned int counter_hi;
  volatile unsigned int compare[4];
};

#define REGS_TIMER ((struct timer_regs *)(MMIO::PERIPHERAL_BASE + 0x00003000))

struct arm_irq_regs_2711 {
  volatile unsigned int irq0_pending_0;
  volatile unsigned int irq0_pending_1;
  volatile unsigned int irq0_pending_2;
  volatile unsigned int res0;
  volatile unsigned int irq0_enable_0;
  volatile unsigned int irq0_enable_1;
  volatile unsigned int irq0_enable_2;
  volatile unsigned int res1;
  volatile unsigned int irq0_disable_0;
  volatile unsigned int irq0_disable_1;
  volatile unsigned int irq0_disable_2;
};

typedef struct arm_irq_regs_2711 arm_irq_regs;

#define REGS_IRQ ((arm_irq_regs *)(MMIO::PERIPHERAL_BASE + 0x0000B200))

enum vc_irqs {
  SYS_TIMER_IRQ_0 = 1,
  SYS_TIMER_IRQ_1 = 2,
  SYS_TIMER_IRQ_2 = 4,
  SYS_TIMER_IRQ_3 = 8,
  AUX_IRQ = (1 << 29),
  AUX_IRQ2 = (1 << 30)

};

void enable_interrupt_controller() {
  REGS_IRQ->irq0_enable_0 =
      SYS_TIMER_IRQ_1 | SYS_TIMER_IRQ_3 | AUX_IRQ | AUX_IRQ2;
}

void disable_interrupt_controller() { REGS_IRQ->irq0_enable_0 = 0; }

#define CLOCKHZ 1000000

const unsigned int timer1_int = CLOCKHZ;
const unsigned int timer3_int = CLOCKHZ / 4;

unsigned int timer1_val = 0;
unsigned int timer3_val = 0;

void timer_init() {
  timer1_val = REGS_TIMER->counter_lo;
  timer1_val += timer1_int;
  REGS_TIMER->compare[1] = timer1_val;

  timer3_val = REGS_TIMER->counter_lo;
  timer3_val += timer3_int;
  REGS_TIMER->compare[3] = timer3_val;
}

void wait_msec(unsigned int n) {
  register unsigned int count = REGS_TIMER->counter_lo;
  REGS_TIMER->compare[1] = count + (n * 1000);
  unsigned int comp = REGS_TIMER->compare[1];
  do {
    count = REGS_TIMER->counter_lo;
  } while (count < comp);
}

void handle_timer_1() {
  Console::print("Timer1");

  timer1_val += timer1_int;
  REGS_TIMER->compare[1] = timer1_val;
  REGS_TIMER->control_status |= SYS_TIMER_IRQ_1;

  unsigned int progval = timer1_val / timer1_int;
  if (progval <= 100) {
    Console::print("x");
  } else {
    Console::print("!\n");
  }
}

void handle_timer_3() {
  Console::print("Timer3");

  timer3_val += timer3_int;
  REGS_TIMER->compare[3] = timer3_val;
  REGS_TIMER->control_status |= SYS_TIMER_IRQ_3;

  unsigned int progval = timer3_val / timer3_int;
  if (progval <= 100) {
    Console::print("x");
  } else {
    Console::print("!\n");
  }
}

extern "C" void handle_error() {
  unsigned int irq = REGS_IRQ->irq0_pending_0;
  Console::print("Error! %d\n", irq);
  //_wait_for_event();
}

extern "C" void handle_irq() {
  unsigned int irq = REGS_IRQ->irq0_pending_0;
  Console::print("Exception irq! %d\n", irq);
  while (irq) {
    if (irq & SYS_TIMER_IRQ_1) {
      irq &= ~SYS_TIMER_IRQ_1;

      handle_timer_1();
    }

    if (irq & SYS_TIMER_IRQ_3) {
      irq &= ~SYS_TIMER_IRQ_3;

      handle_timer_3();
    }
  }
}
