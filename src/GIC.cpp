
#include "GIC.h"

#include "Console.h"
#include "IRQ.h"
#include "Mem.h"
#include "SystemTimer.h"

using ltl::console::Console;

typedef struct {
  gic400_gicd_t* gicd;
  gic400_gicc_t* gicc;
} gic400_t;

static gic400_t gic400;

#define GIC_BASE 0xFF840000
#define GICD_DIST_BASE (GIC_BASE + 0x00001000)
#define GICC_CPU_BASE (GIC_BASE + 0x00002000)

#define GICD_ENABLE_IRQ_BASE (GICD_DIST_BASE + 0x00000100)

#define GICC_IAR (GICC_CPU_BASE + 0x0000000C)
#define GICC_EOIR (GICC_CPU_BASE + 0x00000010)

#define GIC_IRQ_TARGET_BASE (GICD_DIST_BASE + 0x00000800)

// VC (=VideoCore) starts at 96
#define SYSTEM_TIMER_IRQ_0 (0x60)  // 96
#define SYSTEM_TIMER_IRQ_1 (0x61)  // 97
#define SYSTEM_TIMER_IRQ_2 (0x62)  // 98
#define SYSTEM_TIMER_IRQ_3 (0x63)  // 99

void print_gic_state() {
  /*
  GICC_CTLR: 00000001 GICC_PMR:    000000f8 GICC_BPR: 00000002
GICC_RPR:  000000ff GICC_HPPIR:  000003ff
GICC_ABPR: 00000003 GICC_AHPPIR: 000003ff

GICD_CTLR:        00000001 GICD_TYPER:      0000fc67 GICD_IGROUPR0:   00000000
GICD_ISENABLER0:  0000ffff GICD_ISPENDR0:   00000000 GICD_ISACTIVER0: 00000000
GICD_IPRIORITYR0: 00000000 GICD_ITARGETSR0: 01010101 GICD_ICFGR0:     aaaaaaaa
GICD_PPISR:       00000000 GICD_SPISR0:     00000000 GICD_SPENSGIR0:  00000000
  */

  uint32_t reg = GICD_DIST_BASE + 8;

  Console::print("GICID 0x%x OTHER: 0x%x\n", gic400.gicd->iid, MMIO::read(reg));
  Console::print(
      "##################\n\nGICC_CTLR: 0x%x  GICC_PMR: 0x%x  GICC_BPR: 0x%x\n",
      gic400.gicc->ctl, gic400.gicc->pm, gic400.gicc->bp);
  Console::print("GICC_RPR: 0x%x  GICC_HPPIR: 0x%x\n", gic400.gicc->rp,
                 gic400.gicc->hppi);
  Console::print("GICC_ABPR: 0x%x  GICC_AHPPIR: 0x%x\n\n", gic400.gicc->abp,
                 gic400.gicc->ahppi);

  Console::print("GICD_CTLR: 0x%x  GICD_TYPE: 0x%x  GICD_IGGRP0: 0x%x\n",
                 gic400.gicd->ctl, gic400.gicd->type, gic400.gicd->igroup[0]);
  Console::print(
      "GICD_ISENABLER0: 0x%x  GICD_ISPEND0: 0x%x  GICD_ISACTIVE: 0x%x\n",
      gic400.gicd->isenable[0], gic400.gicd->ispend[0],
      gic400.gicd->isactive[0]);
  Console::print("GICD_IPRI0: 0x%x  GICD_ITARGET0: 0x%x  GICD_ICFGR0: %x\n",
                 gic400.gicd->ipriority[0], gic400.gicd->istargets[0],
                 gic400.gicd->icfg[0]);
  Console::print("GICD_PPIS: 0x%x  GICD_SPI0: 0x%x  GICD_SPENSGI0: 0x%x\n",
                 gic400.gicd->ppis, gic400.gicd->spis[0],
                 gic400.gicd->spendsgi[0]);
}

void enable_interrupt(unsigned int irq) {
  // Console::print("Enabling: 0x%x\r\n", irq);
  unsigned int n = irq / 32;
  unsigned int offset = irq % 32;
  unsigned int enableRegister = GICD_ENABLE_IRQ_BASE + (4 * n);
  // Console::print("EnableRegister: %x\r\n", enableRegister);
  MMIO::write(enableRegister, 1 << offset);
}

void assign_target(unsigned int irq, unsigned int cpu) {
  unsigned int n = irq / 4;
  unsigned int targetRegister = GIC_IRQ_TARGET_BASE + (4 * n);
  // Currently we only enter the target CPU 0
  // MMIO::write(targetRegister, MMIO::read(targetRegister) | (1 << 8));

  uint32_t byte_offset = irq % 4;
  uint32_t shift = byte_offset * 8 + cpu;
  // GICD_ITARGETSRN->set[n] |= (1 << shift);
  Console::print("Shift %d\n", shift);
  MMIO::write(targetRegister, MMIO::read(targetRegister) | (1 << shift));
}

void gicInit() {
  uint64_t interrupt_controller_base = 0xFF840000UL;
  gic400.gicd = (gic400_gicd_t*)(interrupt_controller_base + 0x1000);
  gic400.gicc = (gic400_gicc_t*)(interrupt_controller_base + 0x2000);

  /* Disable the controller so we can configure it before it passes any
     interrupts to the CPU */
  gic400.gicd->ctl = GIC400_CTL_DISABLE;
  gic400.gicc->ctl = GIC400_CTL_DISABLE;

  Console::print("First Init\n ########################\n");
  print_gic_state();

  MMIO::write(((long)&gic400.gicc->pm), 0x0000FFu);

  gic400.gicc->bp = 2;

  assign_target(SYSTEM_TIMER_IRQ_1, 0);
  enable_interrupt(SYSTEM_TIMER_IRQ_1);

  /*   assign_target(SYSTEM_TIMER_IRQ_3, 2);
    enable_interrupt(SYSTEM_TIMER_IRQ_3); */

  gic400.gicc->ctl = GIC400_CTL_ENABLE;
  gic400.gicd->ctl = GIC400_CTL_ENABLE;
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
unsigned int* _spin = (unsigned int*)0xd8;
extern unsigned int _core_count1;

void spin_msec(unsigned int n) {
  rpi_sys_timer_t* sys_timer = RPI_GetSystemTimer();

  unsigned int target = sys_timer->counter_lo + (n * 1000);
  unsigned int t = sys_timer->counter_lo;

  while (t < target) {
    t = sys_timer->counter_lo;
  }
}

extern "C" void irq_h() {
  disable_irq();
  // Console::print("CORE COUNT1: 0x%d\n", _core_count1);
  // Console::print("CORE: %d EL: %d ", get_core(), get_el());
  unsigned int irq_ack_reg = MMIO::read(GICC_IAR);
  // Console::print("IRQ ACK REQ 0x%x\n", irq_ack_reg);
  unsigned int irq = irq_ack_reg & 0x2FF;
  rpi_sys_timer_t* sys_timer = RPI_GetSystemTimer();
  // print_gic_state();
  switch (irq) {
    case (SYSTEM_TIMER_IRQ_1):
      Console::print("\n\tTimer IRQ 1 Received! Waking up other cores!\n");
      /*        Console::print(
                "CS: 0x%x\nCMP0: 0x%x CMP1: 0x%x CMP2: 0x%x CMP3: 0x%x\nCNTRLO:
         " "0x%x\n\n", sys_timer->control_status, sys_timer->compare0,
         sys_timer->compare1, sys_timer->compare2, sys_timer->compare3,
         sys_timer->counter_lo);
       */      // print_gic_state();
      MMIO::write(GICC_EOIR, irq_ack_reg);
      RPI_GetSystemTimer()->control_status |= 0b0010;
      RPI_WaitMicroSecondsT1(1000000);
      break;

    case (SYSTEM_TIMER_IRQ_3):
      /* Console::print("Timer IRQ 3 Received!\n");
      Console::print(
          "CS: 0x%x\nCMP0: 0x%x CMP1: 0x%x CMP2: 0x%x CMP3: 0x%x\nCNTRLO: "
          "0x%x\n\n",
          sys_timer->control_status, sys_timer->compare0, sys_timer->compare1,
          sys_timer->compare2, sys_timer->compare3, sys_timer->counter_lo); */
      // print_gic_state();
      MMIO::write(GICC_EOIR, irq_ack_reg);
      RPI_GetSystemTimer()->control_status |= 0b1000;
      RPI_WaitMicroSecondsT3(10000000);
      break;

    default:
      Console::print("Unknown pending irq: %x\r\n", irq);
      break;
  }

  enable_irq();
  asm volatile("sev");
}
