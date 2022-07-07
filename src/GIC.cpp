
#include "GIC.h"

#include "Console.h"
#include "IRQ.h"
#include "Mem.h"
#include "SystemTimer.h"

using namespace ltl::console;

typedef struct {
  gic400_gicd_t* gicd;
  gic400_gicc_t* gicc;
} gic400_t;

static gic400_t gic400;

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
  Console::print("GICC_CTLR: 0x%x  GICC_PMR: 0x%x  GICC_BPR: 0x%x\n",
                 gic400.gicc->ctl, gic400.gicc->pm, gic400.gicc->bp);
  Console::print("GICC_RPR: 0x%x  GICC_HPPIR: 0x%x\n", gic400.gicc->rp,
                 gic400.gicc->hppi);
  Console::print("GICC_ABPR: 0x%x  GICC_AHPPIR: 0x%x\n\n\n", gic400.gicc->abp,
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

void enable_interrupt(unsigned int irq) {
  //Console::print("Enabling: 0x%x\r\n", irq);
  unsigned int n = irq / 32;
  unsigned int offset = irq % 32;
  unsigned int enableRegister = GICD_ENABLE_IRQ_BASE + (4 * n);
  //Console::print("EnableRegister: %x\r\n", enableRegister);
  MMIO::write(enableRegister, 1 << offset);
}

void assign_target(unsigned int irq, unsigned int cpu) {
  unsigned int n = irq / 4;
  unsigned int targetRegister = GIC_IRQ_TARGET_BASE + (4 * n);
  // Currently we only enter the target CPU 0
  MMIO::write(targetRegister, MMIO::read(targetRegister) | (1 << 8));
}

void enable_interrupt_controller() {
  assign_target(SYSTEM_TIMER_IRQ_1, 0);
  enable_interrupt(SYSTEM_TIMER_IRQ_1);
}

void new_gic_init() {
  /*  gic400_init((void *)0xFF840000UL); */

  for (int i = 0; i < 100; ++i) {
    enable_interrupt(i);
    assign_target(i, 0);
    // enable_interrupt_controller();
  }
  // enable_interrupt_controller();
  /*  enable_interrupt(3);
   assign_target(3, 0);
   enable_interrupt(1);
   assign_target(1, 0); */
}

extern "C" void irq_h() {
  disable_irq();

  unsigned int irq_ack_reg = MMIO::read(GICC_IAR);
  unsigned int irq = irq_ack_reg & 0x2FF;
  rpi_sys_timer_t* sys_timer = RPI_GetSystemTimer();

  switch (irq) {
    case (SYSTEM_TIMER_IRQ_1):

      // handle_timer_irq();
      Console::print("Timer IRQ Received!\n");
      Console::print(
          "CS: 0x%x\nCMP0: 0x%x CMP1: 0x%x CMP2: 0x%x CMP3: 0x%x\nCNTRLO: "
          "0x%x\n\n",
          sys_timer->control_status, sys_timer->compare0, sys_timer->compare1,
          sys_timer->compare2, sys_timer->compare3, sys_timer->counter_lo);
      // print_gic_state();
      MMIO::write(GICC_EOIR, irq_ack_reg);
      RPI_GetSystemTimer()->control_status = 0b0010;
      RPI_WaitMicroSeconds(2000000);
      break;
    default:
      Console::print("Unknown pending irq: %x\r\n", irq);
      break;
  }

  enable_irq();
}

void gic400_init(void* interrupt_controller_base) {
  int number_of_interrupts = 0;

  gic400.gicd = (gic400_gicd_t*)(interrupt_controller_base + 0x1000);
  gic400.gicc = (gic400_gicc_t*)(interrupt_controller_base + 0x2000);

  /* Disable the controller so we can configure it before it passes any
     interrupts to the CPU */
  gic400.gicd->ctl = GIC400_CTL_DISABLE;
  gic400.gicc->ctl = GIC400_CTL_DISABLE;

  Console::print("FIrst Init ########################\n");
  print_gic_state();

  MMIO::write(((long)&gic400.gicc->pm), 0x0000FFu);

  // gic400.gicc->pm = 0xFF;
  gic400.gicc->bp = 2;

  Console::print("IID: 0x%x\n", gic400.gicd->iid);
  /* Get the number of interrupt lines implemented in the GIC400 controller */
  number_of_interrupts = GIC400_TYPE_ITLINESNUMBER_GET(gic400.gicd->type) * 32;
  Console::print("Supporting %d ints\n", number_of_interrupts);
  /* The actual number returned by the ITLINESNUMBER is the number of registered
     implemented. The actual number of interrupt lines available is
     (ITLINESNUMBER * 32) */

  for (unsigned i = 0; i < (number_of_interrupts >> 5); ++i) {
    unsigned offs = i << 2;
    if (!i) {
      // CPU::MMIOWrite32(distrAddr + GICD_ISENABLERn + offs, 0x0000FFFFu);
      MMIO::write(((long)&gic400.gicd->isenable + offs), 0x0000FFFFu);
      // CPU::MMIOWrite32(distrAddr + GICD_ICENABLERn + offs, 0xFFFF0000u);
      // gic400.gicd->icenable[offs] = 0xFFFF0000u;
      MMIO::write(((long)&gic400.gicd->icenable + offs), 0xFFFF0000u);

    } else
      // CPU::MMIOWrite32(distrAddr + GICD_ICENABLERn + offs, 0xFFFFFFFFu);
      // gic400.gicd->icenable[offs] = 0xFFFFFFFFu;
      MMIO::write(((long)&gic400.gicd->icenable + offs), 0xFFFFFFFFu);

    MMIO::write(((long)&gic400.gicd->icpend + offs), 0xFFFFFFFFu);
    MMIO::write(((long)&gic400.gicd->icactive + offs), 0xFFFFFFFFu);
    MMIO::write(((long)&gic400.gicd->igroup + offs), 0);
    /*
          gic400.gicd->icpend[offs] = 0xFFFFFFFFu;
         gic400.gicd->icactive[offs] = 0xFFFFFFFFu;
         gic400.gicd->igroup[offs] = 0;  */

    /*   CPU::MMIOWrite32(distrAddr + GICD_ICPENDRn + offs, 0xFFFFFFFFu);
      CPU::MMIOWrite32(distrAddr + GICD_ICACTIVERn + offs, 0xFFFFFFFFu);
      CPU::MMIOWrite32(distrAddr + GICD_IGROUPRn + offs, 0u); */
  }

  for (unsigned i = 0; i < (number_of_interrupts >> 2); ++i) {
    gic400.gicd->ipriority[i << 2] = 0x00000000;
    gic400.gicd->istargets[i << 2] = 0x01010101;

    /*   MMIO::write(((long)&gic400.gicd->ipriority + (i << 2)), 0);
      MMIO::write(((long)&gic400.gicd->istargets + (i << 2)), 0x01010101); */

    /* CPU::MMIOWrite32(distrAddr + GICD_IPRIORITYRn + (i << 2), 0x00000000);
    CPU::MMIOWrite32(distrAddr + GICD_ITARGETSRn + (i << 2), 0x01010101); */
  }

  for (unsigned i = 0; i < (number_of_interrupts >> 4); ++i)
    MMIO::write(((long)&gic400.gicd->icfg + (i << 2)), 0);
  //  gic400.gicd->icfg[i << 2] = 0x00000000;
  gic400.gicc->pm = 0xFF;

  // CPU::MMIOWrite32(distrAddr + GICD_ICFGRn + (i << 2), 0);

  gic400.gicc->ctl = GIC400_CTL_ENABLE;
  gic400.gicd->ctl = GIC400_CTL_ENABLE;
}
