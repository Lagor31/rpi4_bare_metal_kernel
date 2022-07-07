#include "IRQ.h"

#include <stddef.h>
#include <stdint.h>

#include "ARMTimer.h"
#include "Console.h"
#include "GIC.h"
#include "LegacyInt.h"
#include "Mem.h"
#include "boot/sysregs.h"
#include "stdlib/Stdlib.h"

extern "C" void _enable_interrupts();

using namespace ltl::console;
//#define RPI 4
// Memory-Mapped I/O output
static inline void mmio_write(intptr_t reg, uint32_t data) {
  *(volatile uint32_t *)reg = data;
}

// Memory-Mapped I/O input
static inline uint32_t mmio_read(intptr_t reg) {
  return *(volatile uint32_t *)reg;
}
static inline void io_halt(void) { asm volatile("wfi"); }

extern void enable_irq(void);
extern void disable_irq(void);

static unsigned int cntfrq = 0;

void enable_cntv(void) {
  unsigned int cntv_ctl;
  cntv_ctl = 1;
  asm volatile("msr cntv_ctl_el0, %0" ::"r"(cntv_ctl));
}

void disable_cntv(void) {
  unsigned int cntv_ctl;
  cntv_ctl = 0;
  asm volatile("msr cntv_ctl_el0, %0" ::"r"(cntv_ctl));
}

unsigned long read_cntvct(void) {
  unsigned long val;
  asm volatile("mrs %0, cntvct_el0" : "=r"(val));
  return (val);
}

unsigned int read_cntv_tval(void) {
  unsigned int val;
  asm volatile("mrs %0, cntv_tval_el0" : "=r"(val));
  return val;
}

void write_cntv_tval(unsigned int val) {
  asm volatile("msr cntv_tval_el0, %0" ::"r"(val));
  return;
}

unsigned int read_cntfrq(void) {
  unsigned int val;
  asm volatile("mrs %0, cntfrq_el0" : "=r"(val));
  return val;
}

#if RPI == 3
#define LOCAL_TIMER_BASE 0x40000000
#define CORE0_TIMER_IRQCNTL 0x40000040
#define CORE0_IRQ_SOURCE 0x40000060

#else
#define LOCAL_TIMER_BASE 0xFF800000
#define CORE0_TIMER_IRQCNTL 0xFF800040
#define CORE0_IRQ_SOURCE 0xFF800060

#endif

void routing_core0cntv_to_core0irq(void) {
  mmio_write(CORE0_TIMER_IRQCNTL, 0x08 | 1);
}

unsigned int read_core0timer_pending(void) {
  unsigned int tmp;
  tmp = mmio_read(CORE0_IRQ_SOURCE);
  return tmp;
}




extern "C" void c_irq_handler(void) {

  Console::print("IRQ!\n");
  print_gic_state();
  unsigned int cntvct;
  unsigned int val;

  disable_irq();
  if (read_core0timer_pending() & 0x08) {
    Console::print("handler CNTV_TVAL: ");
    val = read_cntv_tval();
    Console::print("0x%x\n", val);

    write_cntv_tval(cntfrq);  // clear cntv interrupt and set next 1sec timer.
    cntvct = read_cntvct();
    Console::print("handler CNTVCT   : 0x%x\n", cntvct);
    Console::print("handler CNTVCT TVAL   : 0x%x\n", read_cntv_tval);
  }

  enable_irq();
  return;
}

void raw_write_daif(uint32_t daif) {
  __asm__ __volatile__("msr DAIF, %0\n\t" : : "r"(daif) : "memory");
}

void timer_init() {
  /* cntfrq = read_cntfrq();
  write_cntv_tval(cntfrq);  // clear cntv interrupt and set next 1 sec timer.
  Console::print("CNTV_TVAL: 0x%x\n", read_cntv_tval());
  routing_core0cntv_to_core0irq(); */

  //gic400_init((void *)0xFF840000UL);

  new_gic_init();
  // enable_cntv();

  enable_irq();

  // raw_write_daif(2);

  //_enable_interrupts();
}
