#ifndef IRQ_H
#define IRQ_H

#include <stddef.h>
#include <stdint.h>
#include "Mem.h"
#include "Spinlock.h"

#include "Task.h"


uint32_t calcNextCore(Task* t);

extern "C" void enable_irq();
extern "C" void disable_irq();
// void disable_interrupt_controller();
void handle_irq();
void initSchedLock();

struct core_context {
  uint64_t x19;
  uint64_t x20;
  uint64_t x21;
  uint64_t x22;
  uint64_t x23;
  uint64_t x24;
  uint64_t x25;
  uint64_t x26;
  uint64_t x27;
  uint64_t x28;
  uint64_t fp;
  uint64_t sp;
  uint64_t lr;
};



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

#endif