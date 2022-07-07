#include "IRQ.h"

#include <stddef.h>
#include <stdint.h>

#include "Console.h"
#include "GIC.h"
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

#if RPI == 3
#define LOCAL_TIMER_BASE 0x40000000
#define CORE0_TIMER_IRQCNTL 0x40000040
#define CORE0_IRQ_SOURCE 0x40000060

#else
#define LOCAL_TIMER_BASE 0xFF800000
#define CORE0_TIMER_IRQCNTL 0xFF800040
#define CORE0_IRQ_SOURCE 0xFF800060

#endif

void raw_write_daif(uint32_t daif) {
  __asm__ __volatile__("msr DAIF, %0\n\t" : : "r"(daif) : "memory");
}

void timer_init() {
  disable_irq();
  gicInit();
  enable_irq();
}
