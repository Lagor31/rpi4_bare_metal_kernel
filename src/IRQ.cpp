#include "IRQ.h"

#include <stddef.h>
#include <stdint.h>

#include "Console.h"
#include "GIC.h"
#include "Lock.h"
#include "Mem.h"
#include "SystemTimer.h"
#include "boot/sysregs.h"
#include "stdlib/Stdlib.h"

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

static splck_t sched_lock;

void timerInit() {
  disable_irq();
  splck_init(&sched_lock);
  gicInit();
  RPI_WaitMicroSecondsT1(1000000);
  RPI_WaitMicroSecondsT3(1000000);
  Console::print("Timer init on core: %d\n", get_core());
  enable_irq();
}
uint64_t core_activations[4] = {0};

// Current EL with SPx
extern "C" void irq_handler_spx() {
  disable_irq();

  // Console::print("CORE: %d EL: %d ", get_core(), get_el());

  /*  Console::print(
       "\nReceived IRQ Exception\n\tCore: %d IRQ: %d From: Core%d ID: %d\n",
       get_core(), irq, cpu, c++); */
  // Console::print("Current Level with SPX\n");
  splck_lck(&sched_lock);
  unsigned int irq_ack_reg = MMIO::read(GICC_IAR);
  // Console::print("IRQ ACK REQ 0x%x\n", irq_ack_reg);
  unsigned int irq = irq_ack_reg & 0x3FF;
  unsigned int cpu = (irq_ack_reg >> 10) & 7;
  rpi_sys_timer_t *sys_timer = RPI_GetSystemTimer();
  splck_done(&sched_lock);

  //   print_gic_state();
  switch (irq) {
    case (SYSTEM_TIMER_IRQ_1):

      Console::print(
          "\nTimer IRQ 1\n\tCore%d IRQ: %d From: Core%d\n"
          "\tC0: %u C1:%u C2:%u C3:%u\n",
          get_core(), irq, cpu, core_activations[0], core_activations[1],
          core_activations[2], core_activations[3]);

      splck_lck(&sched_lock);
      /*    if ((sys_timer->compare0 % 3) == 0)
           send_sgi(2, 3);
         else */
      // if ((sys_timer->compare2 % 3) == 0) send_sgi(2, 1);
      /* if ((sys_timer->compare2 % 2) == 0)
        send_sgi(2, 2);
      else
        send_sgi(2, 1); */
      // send_sgi(2, 2);

      RPI_WaitMicroSecondsT1(20000);
      RPI_GetSystemTimer()->control_status |= 0b0010;
      MMIO::write(GICC_EOIR, irq);
      splck_done(&sched_lock);
      break;

    case (SYSTEM_TIMER_IRQ_3):
      Console::print(
          "\nTimer IRQ 3\n\tCore%d IRQ: %d From: Core%d\n"
          "\tC0: %u C1:%u C2:%u C3:%u\n",
          get_core(), irq, cpu, core_activations[0], core_activations[1],
          core_activations[2], core_activations[3]);

      splck_lck(&sched_lock);
      if ((sys_timer->compare2 % 2) == 0)
        send_sgi(2, 2);
     /*  else
        send_sgi(2, 1); */

      RPI_GetSystemTimer()->control_status |= 0b1000;
      RPI_WaitMicroSecondsT3(50000);
      MMIO::write(GICC_EOIR, irq);
      splck_done(&sched_lock);

      break;
    case 1023:
      Console::print("SPOURIOUS INT RECEIVED: %x\r\n", irq);
      splck_lck(&sched_lock);
      MMIO::write(GICC_EOIR, irq);
      splck_done(&sched_lock);
      break;
    default:
      /* spin_msec(get_core() * 5 + 50);
      Console::print("Unknown pending irq: %x from core: %d\r\n", irq, cpu); */

      /*  Console::print(
          "Received IRQ Exception\n\tCore%d IRQ: %d From: Core%d\n"
          "\tC0: %u C1:%u C2:%u C3:%u\n",
          get_core(), irq, cpu, core_activations[0], core_activations[1],
          core_activations[2], core_activations[3]);
      */
      splck_lck(&sched_lock);
      MMIO::write(GICC_EOIR, irq);
      splck_done(&sched_lock);

      break;
  }
  // Console::print("\n");
  splck_lck(&sched_lock);
  core_activations[get_core()]++;
  splck_done(&sched_lock);

  enable_irq();
}

extern "C" void sync_handler_spx() {
  unsigned long add = get_far_el1();
  unsigned long cause = get_esr_el1();
  unsigned long ret = get_elr_el1();
  Console::print_no_lock("Received SYNC Exception on core %d!!!\n", get_core());
  Console::print_no_lock("Current Level with SPX\n");
  Console::print_no_lock("Fault address: 0x%x\n", add);
  Console::print_no_lock("Cause address: 0x%x\n", cause);
  Console::print_no_lock("Ret add address: 0x%x\n", ret);
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
  print_gic_state();
  Console::print_no_lock("Panicking on Core %d!\n", get_core());
  unsigned int irq_ack_reg = MMIO::read(GICC_IAR);
  unsigned int irq = irq_ack_reg & 0x2FF;
  Console::print_no_lock("IRQ: 0x%d\n", irq);
  MMIO::write(GICC_EOIR, irq_ack_reg);
  // enable_irq();
}
