
#include "GIC.h"

#include "Console.h"
#include "IRQ.h"
#include "Mem.h"
#include "SystemTimer.h"

using ltl::console::Console;

gic400_t gic400;

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

  uint64_t reg = GICD_DIST_BASE + 8;

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
  uint64_t enableRegister = GICD_ENABLE_IRQ_BASE + (4 * n);
  // Console::print("EnableRegister: %x\r\n", enableRegister);
  MMIO::write(enableRegister, 1 << offset);
}

void assign_target(unsigned int irq, unsigned int cpu) {
  unsigned int n = irq / 4;
  uint64_t targetRegister = GIC_IRQ_TARGET_BASE + (4 * n);
  uint32_t byte_offset = irq % 4;
  uint32_t shift = byte_offset * 8 + cpu;
  Console::print("Shift %d\n", shift);
  MMIO::write(targetRegister, MMIO::read(targetRegister) | (1 << shift));
}
void send_sgi(unsigned int irq, unsigned int cpu) {
  MMIO::write(GICD_SGIR, (1 << (16 + cpu) | irq));
  // Console::print("SGIR: 0x%x\n", gic400.gicd->sgi);
}

void gicInit() {
  uint64_t interrupt_controller_base = 0xffff000000000000 + 0xFF840000UL;
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
  // gic400.gicd->sgi |= (1 << 24);

  assign_target(SYSTEM_TIMER_IRQ_1, 0);
  enable_interrupt(SYSTEM_TIMER_IRQ_1);

  assign_target(SYSTEM_TIMER_IRQ_3, 3);
  enable_interrupt(SYSTEM_TIMER_IRQ_3);

  assign_target(1, 0);
  assign_target(1, 1);
  assign_target(1, 2);
  assign_target(1, 3);

  enable_interrupt(1);

  gic400.gicc->ctl = GIC400_CTL_ENABLE;
  gic400.gicd->ctl = GIC400_CTL_ENABLE;
}

void spin_msec(unsigned int n) {
  rpi_sys_timer_t* sys_timer = RPI_GetSystemTimer();

  unsigned int target = sys_timer->counter_lo + (n * 1000);
  unsigned int t = sys_timer->counter_lo;

  while (t < target) {
    t = sys_timer->counter_lo;
  }
}
