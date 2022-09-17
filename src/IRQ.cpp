#include "include/IRQ.h"

#include <stddef.h>
#include <stdint.h>

#include "include/Console.h"
#include "include/GIC.h"
#include "include/Lock.h"
#include "include/Mem.h"
#include "include/Stdlib.h"
#include "include/SystemTimer.h"
#include "include/sysregs.h"

extern "C" void _enable_interrupts();
extern "C" uint64_t get_far_el1();
extern "C" uint64_t get_esr_el1();
extern "C" uint64_t get_elr_el1();

void enable_interrupt_controller() {
  REGS_IRQ->irq0_enable_0 =
      SYS_TIMER_IRQ_1 | SYS_TIMER_IRQ_3 | AUX_IRQ | AUX_IRQ2;
}
void disable_interrupt_controller() { REGS_IRQ->irq0_enable_0 = 0; }

using ltl::console::Console;

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

void raw_write_daif(uint32_t daif) {
  __asm__ __volatile__("msr DAIF, %0\n\t" : : "r"(daif) : "memory");
}

// Current level w/ SP0
extern "C" void irq_handler_sp0() {
  Console::print("Received IRQ Exception on core %d!!!\n", get_core());
  Console::print("Current Level with SP0\n");
  _hang_forever();
}

extern "C" void sync_handler_sp0() {
  Console::print("Received SYNC Exception on core %d!!!\n", get_core());
  Console::print("Current Level with SP0\n");
  _hang_forever();
}

extern "C" void fiq_handler_sp0() {
  Console::print("Received FIQ Exception on core %d!!!\n", get_core());
  Console::print("Current Level with SP0\n");
  _hang_forever();
}

extern "C" void serror_handler_sp0() {
  Console::print("Received SEError Exception on core %d!!!\n", get_core());
  Console::print("Current Level with SP0\n");
  _hang_forever();
}

splck_t sched_lock;

void init_sched() { splck_init(&sched_lock); }

void timerInit() {
  disable_irq();
  SystemTimer::WaitMicroT1(100000);
  SystemTimer::WaitMicroT3(200000);
  Console::print("Timer init on core: %d\n", get_core());
  enable_irq();
}
uint64_t core_activations[4] = {0};

// Current EL with SPx
extern "C" void irq_handler_spx(irq_regs_t regs) {
  unsigned int irq_ack_reg = MMIO::read(GICC_IAR);
  unsigned int irq = irq_ack_reg & 0x3FF;
  unsigned int cpu = (irq_ack_reg >> 10) & 7;
  uint64_t core_activations_l[4];
  switch (irq) {
    case (SYSTEM_TIMER_IRQ_1):
      MMIO::write(GICC_EOIR, irq);

      splck_lck(&sched_lock);
      for (int i = 0; i < 4; ++i) core_activations_l[i] = core_activations[i];
      splck_done(&sched_lock);

      Console::print(
          "\nTimer IRQ 1\n\tCore%d IRQ: %d From: Core%d\n"
          "\tC0: %u C1:%u C2:%u C3:%u\n",
          get_core(), irq, cpu, core_activations_l[0], core_activations_l[1],
          core_activations_l[2], core_activations_l[3]);
      splck_lck(&sched_lock);
      SystemTimer::WaitMicroT1(200000);
      SystemTimer::getTimer()->control_status |= 0b0010;
      splck_done(&sched_lock);

      if ((core_activations_l[0] % 10) == 0) GIC400::send_sgi(2, 1);

      break;

    case (SYSTEM_TIMER_IRQ_3):
      MMIO::write(GICC_EOIR, irq);
      splck_lck(&sched_lock);
      for (int i = 0; i < 4; ++i) core_activations_l[i] = core_activations[i];
      splck_done(&sched_lock);

      Console::print(
          "\nTimer IRQ 3\n\tCore%d IRQ: %d From: Core%d\n"
          "\tC0: %u C1:%u C2:%u C3:%u\n",
          get_core(), irq, cpu, core_activations_l[0], core_activations_l[1],
          core_activations_l[2], core_activations_l[3]);

      splck_lck(&sched_lock);
      SystemTimer::getTimer()->control_status |= 0b1000;
      SystemTimer::WaitMicroT3(40000);

      splck_done(&sched_lock);
      if ((core_activations_l[3] % 20) == 0) GIC400::send_sgi(2, 2);
      break;
    case 1023:
      Console::print("SPOURIOUS INT RECEIVED Core%d: %x\r\n", get_core(), irq);
      MMIO::write(GICC_EOIR, irq);
      break;
    default:
      MMIO::write(GICC_EOIR, irq);
      splck_lck(&sched_lock);
      for (int i = 0; i < 4; ++i) core_activations_l[i] = core_activations[i];
      splck_done(&sched_lock);
      Console::print(
          "Received IRQ Exception\n\tCore%d IRQ: %d From: Core%d\n"
          "\tC0: %u C1:%u C2:%u C3:%u\n",
          get_core(), irq, cpu, core_activations_l[0], core_activations_l[1],
          core_activations_l[2], core_activations_l[3]);
      break;
  }
  splck_lck(&sched_lock);
  core_activations[get_core()]++;
  splck_done(&sched_lock);
}

extern "C" void sync_handler_spx(irq_regs_t *regs) {
  unsigned long add = get_far_el1();
  unsigned long cause = get_esr_el1();
  unsigned long ret = get_elr_el1();
  while (true) {
    Console::print_no_lock("Received SYNC Exception on core %d!!!\n",
                           get_core());
    Console::print_no_lock("Current Level with SPX\n");
    Console::print_no_lock("ELR_EL1: 0x%x - 0x%x\n", regs->elr_el1, ret);
    Console::print_no_lock("FAR_EL1: 0x%x\n", add);
    Console::print_no_lock("ESR_EL1: 0x%x - 0x%x\n", regs->esr_el1, cause);
    Console::print_no_lock("SPRS_EL1: 0x%x\n", regs->sprs_el1);
    Console::print_no_lock("LR:       0x%x\n", regs->lr);

    spin_msec(1000 + get_core() * 10);
  }
  _hang_forever();
}

extern "C" void fiq_handler_spx() {
  Console::print("Received FIQ Exception on core %d!!!\n", get_core());
  Console::print("Current Level with SPX\n");
  _hang_forever();
}

extern "C" void serror_handler_spx() {
  Console::print("Received SEError Exception on core %d!!!\n", get_core());
  Console::print("Current Level with SPX\n");
  _hang_forever();
}

// LowerEL ausing AArch64

extern "C" void irq_handler_lower_aarch64() {
  Console::print("Received IRQ Exception on core %d!!!\n", get_core());
  Console::print("Lower level with aarch64\n");
  _hang_forever();
}
extern "C" void sync_handler_lower_aarch64() {
  Console::print("Received SYNC Exception on core %d!!!\n", get_core());
  Console::print("Lower level with aarch64\n");
  _hang_forever();
}

extern "C" void fiq_handler_lower_aarch64() {
  Console::print("Received FIQ Exception on core %d!!!\n", get_core());
  Console::print("Lower level with aarch64\n");
  _hang_forever();
}

extern "C" void serror_handler_lower_aarch64() {
  Console::print("Received SEError Exception on core %d!!!\n", get_core());
  Console::print("Lower level with aarch64\n");
  _hang_forever();
}

// LowerEL ausing AArch32

extern "C" void irq_handler_lower_aarch32() {
  Console::print("Received IRQ Exception on core %d!!!\n", get_core());
  Console::print("Lower level with aarch32\n");
  _hang_forever();
}
extern "C" void sync_handler_lower_aarch32() {
  Console::print("Received SYNC Exception on core %d!!!\n", get_core());
  Console::print("Lower level with aarch32\n");
  _hang_forever();
}

extern "C" void fiq_handler_lower_aarch32() {
  Console::print("Received FIQ Exception on core %d!!!\n", get_core());
  Console::print("Lower level with aarch32\n");
  _hang_forever();
}

extern "C" void serror_handler_lower_aarch32() {
  Console::print("Received SEError Exception on core %d!!!\n", get_core());
  Console::print("Lower level with aarch32\n");
  _hang_forever();
}

extern "C" void panic() {
  disable_irq();
  Console::print_no_lock("Panicking on Core %d!\n", get_core());
  unsigned int irq_ack_reg = MMIO::read(GICC_IAR);
  unsigned int irq = irq_ack_reg & 0x2FF;
  Console::print_no_lock("IRQ: 0x%d\n", irq);
  MMIO::write(GICC_EOIR, irq_ack_reg);
  // enable_irq();
}
