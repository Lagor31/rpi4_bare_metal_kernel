#include "IRQ.h"

#include <stddef.h>
#include <stdint.h>

#include "Console.h"
#include "GIC.h"
#include "Mem.h"
#include "SystemTimer.h"
#include "boot/sysregs.h"
#include "stdlib/Stdlib.h"

extern "C" void _enable_interrupts();

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
void timerInit() {
  disable_irq();
  gicInit();
  RPI_WaitMicroSecondsT1(1000000);
  // RPI_WaitMicroSecondsT3(10000000);
  Console::print("Timer init on core: %d\n", get_core());
  enable_irq();
}

uint64_t c = 0;
// Current EL with SPx
extern "C" void irq_handler_spx() {
  disable_irq();

  // Console::print("CORE: %d EL: %d ", get_core(), get_el());
  unsigned int irq_ack_reg = MMIO::read(GICC_IAR);
  // Console::print("IRQ ACK REQ 0x%x\n", irq_ack_reg);
  unsigned int irq = irq_ack_reg & 0x3FF;
  unsigned int cpu = (irq_ack_reg >> 10) & 7;
  spin_msec((get_core() + 2) * 10);
  Console::print(
      "\nReceived IRQ Exception\n\tCore: %d IRQ: %d From: Core%d ID: %d\n",
      get_core(), irq, cpu, c++);
  // Console::print("Current Level with SPX\n");
  rpi_sys_timer_t *sys_timer = RPI_GetSystemTimer();
  //   print_gic_state();
  switch (irq) {
    case (SYSTEM_TIMER_IRQ_1):
      // Console::print("\n\tTimer IRQ 1 Received! Waking up other cores!\n");
      /*        Console::print(
                "CS: 0x%x\nCMP0: 0x%x CMP1: 0x%x CMP2: 0x%x CMP3: 0x%x\nCNTRLO:
         " "0x%x\n\n", sys_timer->control_status, sys_timer->compare0,
         sys_timer->compare1, sys_timer->compare2, sys_timer->compare3,
         sys_timer->counter_lo);
       */      // print_gic_state();
      Console::print("Timer IRQ 1\n\t");
      Console::print(
          "CS: 0x%x CMP0: 0x%x CMP1: 0x%x CMP2: 0x%x CMP3: 0x%x\n\tCNTRLO: "
          "0x%x ",
          sys_timer->control_status, sys_timer->compare0, sys_timer->compare1,
          sys_timer->compare2, sys_timer->compare3, sys_timer->counter_lo);
      MMIO::write(GICC_EOIR, irq);
      RPI_GetSystemTimer()->control_status |= 0b0010;
      Console::print("CS: 0x%x", sys_timer->control_status);
      send_sgi(2, 2);
      send_sgi(2, 1);
      RPI_WaitMicroSecondsT1(1000000);
      break;

    case (SYSTEM_TIMER_IRQ_3):
      // Console::print("\nTimer IRQ 3 Received!\n");
      /*  Console::print(
           "CS: 0x%x\nCMP0: 0x%x CMP1: 0x%x CMP2: 0x%x CMP3: 0x%x\nCNTRLO: "
           "0x%x\n\n",
           sys_timer->control_status, sys_timer->compare0, sys_timer->compare1,
           sys_timer->compare2, sys_timer->compare3, sys_timer->counter_lo);
       // print_gic_state(); */
      MMIO::write(GICC_EOIR, irq);
      RPI_GetSystemTimer()->control_status |= 0b1000;
      RPI_WaitMicroSecondsT3(10000000);
      break;
    case 1023:
      Console::print("SPOURIOUS INT RECEIVED: %x\r\n", irq);
      MMIO::write(GICC_EOIR, irq);
      break;
    default:
      // Console::print("Unknown pending irq: %x from core: %d\r\n", irq, cpu);
      MMIO::write(GICC_EOIR, irq);
      break;
  }
  Console::print("\n");
  enable_irq();
}

extern "C" void sync_handler_spx() {
  Console::print("Received SYNC Exception on core %d!!!\n", get_core());
  Console::print("Current Level with SPX\n");
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
  Console::print("Panicking on Core %d!\n", get_core());
  unsigned int irq_ack_reg = MMIO::read(GICC_IAR);
  unsigned int irq = irq_ack_reg & 0x2FF;
  Console::print("IRQ: 0x%d\n", irq);
  MMIO::write(GICC_EOIR, irq_ack_reg);
  // enable_irq();
}
