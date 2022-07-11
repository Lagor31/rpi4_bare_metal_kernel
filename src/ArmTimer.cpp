#include "ArmTimer.h"

#include "SMP.h"

unsigned int timer1_val = 0;
unsigned int timer3_val = 0;
uint32_t cntfrq = 0;

void routing_core0cntv_to_core0irq(void) {
  MMIO::write(CORE0_TIMER_IRQCNTL, 0x08);
}

uint32_t read_core0timer_pending(void) {
  uint32_t tmp;
  tmp = MMIO::read(CORE0_IRQ_SOURCE);
  return tmp;
}

void enable_cntv(void) {
  uint32_t cntv_ctl;
  cntv_ctl = 1;
  asm volatile("msr cntv_ctl_el0, %0" ::"r"(cntv_ctl));
}

void disable_cntv(void) {
  uint32_t cntv_ctl;
  cntv_ctl = 0;
  asm volatile("msr cntv_ctl_el0, %0" ::"r"(cntv_ctl));
}

uint64_t read_cntvct(void) {
  uint64_t val;
  asm volatile("mrs %0, cntvct_el0" : "=r"(val));
  return (val);
}

uint32_t read_cntv_tval(void) {
  uint32_t val;
  asm volatile("mrs %0, cntv_tval_el0" : "=r"(val));
  return val;
}

void write_cntv_tval(uint32_t val) {
  asm volatile("msr cntv_tval_el0, %0" ::"r"(val));
  return;
}

uint32_t read_cntfrq(void) {
  uint32_t val;
  asm volatile("mrs %0, cntfrq_el0" : "=r"(val));
  return val;
}

void armTimerInit() {
  cntfrq = read_cntfrq();
  Console::print("CNTFRQ: 0x%x\n", cntfrq);
  write_cntv_tval(cntfrq);
  routing_core0cntv_to_core0irq();
  enable_cntv();
  enable_irq();
}



void wait_msec(unsigned int n) {
  register unsigned int count = REGS_TIMER->counter_lo;
  REGS_TIMER->compare[1] = count + (n * 1000);
  Console::print(
      "CS: 0x%x\nCMP0: 0x%x CMP1: 0x%x CMP2: 0x%x CMP3: 0x%x\nCNTRLO: "
      "0x%x\n\n",
      REGS_TIMER->control_status, REGS_TIMER->compare[0],
      REGS_TIMER->compare[1], REGS_TIMER->compare[2], REGS_TIMER->compare[3],
      REGS_TIMER->counter_lo);
}

#if RPI == 3

extern "C" void irq_h() {
  uint32_t cntvct;
  disable_irq();
  if (read_core0timer_pending() & 0x08) {
    Console::print("IRQ Timer\n");
    write_cntv_tval(cntfrq);  // clear cntv interrupt and set next 1sec timer.
    cntvct = read_cntvct();
    Console::print("CNTVCT: 0x%x\n", cntvct);
  }

  enable_irq();
}

#endif