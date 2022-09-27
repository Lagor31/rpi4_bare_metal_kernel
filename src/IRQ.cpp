#include "include/IRQ.h"

#include <stddef.h>
#include <stdint.h>

#include "include/Console.h"
#include "include/Core.h"
#include "include/GIC.h"
#include "include/Mem.h"
#include "include/Spinlock.h"
#include "include/Stdlib.h"
#include "include/SystemTimer.h"
#include "include/Vector.h"
#include "include/sysregs.h"

extern "C" void _enable_interrupts();
extern "C" uint64_t get_far_el1();
extern "C" uint64_t get_esr_el1();
extern "C" uint64_t get_elr_el1();
extern void kernelThread();
extern "C" uint64_t get_sp();

Spinlock *sched_lock;
void initSchedLock() { sched_lock = new Spinlock(); }
uint64_t core_activations[4] = {0};
uint64_t *handlers[16]{nullptr};

void copyRegs(CoreContext *s, CoreContext *d) {
  for (int i = 0; i < 30; ++i) d->gpr[i] = s->gpr[i];
  d->elr_el1 = s->elr_el1;
  d->esr_el1 = s->esr_el1;
  d->lr = s->lr;
  d->sprs_el1 = s->sprs_el1;
  d->sp_el0 = s->sp_el0;
}
// Current EL with SPx
extern "C" void irq_handler_spx(CoreContext *regs) {
  // Console::print("In IRQH...\n");
  unsigned int irq_ack_reg = MMIO::read(GICC_IAR);
  unsigned int irq = irq_ack_reg & 0x3FF;
  unsigned int cpu = (irq_ack_reg >> 10) & 7;
  sched_lock->getLock();
  uint64_t core_activations_l[4];
  // uint64_t c = core_activations[get_core()] % THREAD_N;

  uint64_t c = Std::djb33_hash(core_activations[get_core()]) % THREAD_N;
  // Vector<Task *> *vnext = Core::runningQ[get_core()];
  Task *next = Core::runningQ[get_core()][c];
  sched_lock->release();
  // Console::print("Out of IRQH...\n");
  switch (irq) {
    case (SYSTEM_TIMER_IRQ_1):
      MMIO::write(GICC_EOIR, irq);
      sched_lock->getLock();
      for (int i = 0; i < 4; ++i) core_activations_l[i] = core_activations[i];
      sched_lock->release();
      SystemTimer::WaitMicroT1(20000);
      SystemTimer::getTimer()->control_status |= 0b0010;

      /*       if (((core_activations_l[0] % 10) == 0) && core_activations_l[0]
         > 0 && core_activations_l[0] < 10000) { Task *t =
         Task::createKernelTask((uint64_t)&kernelThread); uint32_t cpu =
         SystemTimer::getTimer()->counter_lo % 4; Console::print(
                  "####################\nAddint Task to "
                  "Core%d\n####################\n",
                  cpu);
              Core::runningQ[cpu]->push_back(t);
              // GIC400::send_sgi(2, cpu);
            } */

      if (Core::isPreamptable()) {
        // Console::print("Kernel SP: 0x%x\n", get_sp());
        sched_lock->getLock();
        copyRegs(regs, &Core::current[get_core()]->context);
        copyRegs(&next->context, regs);
        Core::current[get_core()] = next;
        if (next->context.elr_el1 < 0xFFFF000000000000) {
          Console::print("Returning to wrong address 0x%x\n",
                         next->context.elr_el1);
          _hang_forever();
        }
        core_activations[get_core()]++;
        sched_lock->release();
      }

      GIC400::send_sgi(2, 1);
      GIC400::send_sgi(2, 2);
      GIC400::send_sgi(2, 3); 
      break;

    case (SYSTEM_TIMER_IRQ_3):
      MMIO::write(GICC_EOIR, irq);
      sched_lock->getLock();
      for (int i = 0; i < 4; ++i) core_activations_l[i] = core_activations[i];
      sched_lock->release();

      /*  Console::print(
           "\nTimer IRQ 3\n\tCore%d IRQ: %d From: Core%d\n"
           "\tC0: %u C1:%u C2:%u C3:%u\n",
           get_core(), irq, cpu, core_activations_l[0], core_activations_l[1],
           core_activations_l[2], core_activations_l[3]);
  */
      SystemTimer::getTimer()->control_status |= 0b1000;
      SystemTimer::WaitMicroT3(20000);
      // core_activations[get_core()]++;

      /*  if (Core::isPreamptable()) {
         // Console::print("Kernel SP: 0x%x\n", get_sp());

         copyRegs(regs, &Core::current[get_core()]->context);
         copyRegs(&vnext[c]->context, regs);
         Core::current[get_core()] = vnext[c];
         if (vnext[c]->context.elr_el1 < 0xFFFF000000000000) {
           Console::print("Returning to wrong address 0x%x\n",
                          vnext[c]->context.elr_el1);
           _hang_forever();
         }
       } */
      if (Core::isPreamptable()) {
        sched_lock->getLock();
        // Console::print("Kernel SP: 0x%x\n", get_sp());
        copyRegs(regs, &Core::current[get_core()]->context);
        copyRegs(&next->context, regs);
        Core::current[get_core()] = next;
        if (next->context.elr_el1 < 0xFFFF000000000000) {
          Console::print("Returning to wrong address 0x%x\n",
                         next->context.elr_el1);
          _hang_forever();
        }
        core_activations[get_core()]++;
        sched_lock->release();
      }

      break;
    case 2:
      MMIO::write(GICC_EOIR, irq);
      if (Core::isPreamptable()) {
        sched_lock->getLock();
        // Console::print("Kernel SP: 0x%x\n", get_sp());
        copyRegs(regs, &Core::current[get_core()]->context);
        copyRegs(&next->context, regs);
        Core::current[get_core()] = next;
        if (next->context.elr_el1 < 0xFFFF000000000000) {
          Console::print("Returning to wrong address 0x%x\n",
                         next->context.elr_el1);
          _hang_forever();
        }
        core_activations[get_core()]++;
        sched_lock->release();
      }

      break;

    case 1:
      Console::print_no_lock("Halting Core%d immediately!\n", get_core());
      _hang_forever();
    case 1023:
      // Console::print("SPOURIOUS INT RECEIVED Core%d: %x\r\n", get_core(),
      // irq);
      MMIO::write(GICC_EOIR, irq);
      break;
    default:
      MMIO::write(GICC_EOIR, irq);
      sched_lock->getLock();
      for (int i = 0; i < 4; ++i) core_activations_l[i] = core_activations[i];
      sched_lock->release();
      /*  Console::print(
           "Received IRQ Exception\n\tCore%d IRQ: %d From: Core%d\n"
           "\tC0: %u C1:%u C2:%u C3:%u\n",
           get_core(), irq, cpu, core_activations_l[0], core_activations_l[1],
           core_activations_l[2], core_activations_l[3]); */
      break;
  }
}

// Current level w/ SP0
extern "C" void irq_handler_sp0(CoreContext *regs) {
  irq_handler_spx(regs);
  /* Console::print_no_lock("Received IRQ Exception on core %d!!!\n",
  get_core()); Console::print_no_lock("Current Level with SP0\n"); */
  //_hang_forever();
}

void printRegs(CoreContext *regs) {
  for (int i = 0; i < 30; ++i)
    Console::print_no_lock("x%d=%x\n", i, regs->gpr[i]);
  Console::print_no_lock(
      "SPSR=%x\nELR=%x\nESR=%x\nLR=%x\nSP_EL0=%x\nFAR_EL1=%x\n", regs->sprs_el1,
      regs->elr_el1, regs->esr_el1, regs->lr, regs->sp_el0, regs->far_el1);
}

extern "C" void sync_handler_sp0(CoreContext *regs) {
  unsigned long add = get_far_el1();
  unsigned long cause = get_esr_el1();
  unsigned long ret = get_elr_el1();

  int i = 0;
  for (; i < 4; ++i) {
    if (get_core() != i) GIC400::send_sgi(1, i);
  }
  i = 0;
  while (i++ < 5) {
    Console::print_no_lock("Received SYNC Exception on core %d!!!\n",
                           get_core());
    Console::print_no_lock("Current Level with SP0\n");
    printRegs(regs);
    Core::spinms(1000 + get_core() * 10);
  }
  _hang_forever();
  // regs->elr_el1 += 4;
  i++;
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

extern "C" void sync_handler_spx(CoreContext *regs) {
  unsigned long add = get_far_el1();
  unsigned long cause = get_esr_el1();
  unsigned long ret = get_elr_el1();
  int i = 0;
  for (; i < 4; ++i) {
    if (get_core() != i) GIC400::send_sgi(1, i);
  }
  i = 0;
  while (i++ < 10) {
    Console::print_no_lock("Received SYNC Exception on core %d!!!\n",
                           get_core());
    Console::print_no_lock("Current Level with SPX\n");
    printRegs(regs);
    Core::spinms(1000 + get_core() * 10);
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
}
