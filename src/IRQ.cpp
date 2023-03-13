#include "include/IRQ.h"

#include <stddef.h>
#include <stdint.h>

#include "include/Console.h"
#include "include/Core.h"
#include "include/GIC.h"
#include "include/KernelHeapAllocator.h"
#include "include/List.h"
#include "include/Mem.h"
#include "include/Spinlock.h"
#include "include/Stdlib.h"
#include "include/Sysregs.h"
#include "include/SystemTimer.h"

uint32_t x = 0;
uint32_t y = 0;
char car = 0;
extern "C" void _enable_interrupts();
extern "C" uint64_t get_far_el1();
extern "C" uint64_t get_esr_el1();
extern "C" uint64_t get_elr_el1();
extern void kernelThread();
extern "C" uint64_t get_sp();

uint32_t nextCore = 0;
Spinlock* sched_lock;
void initSchedLock() { sched_lock = new Spinlock(); }
uint64_t core_activations[4] = {0};
uint64_t* handlers[16]{nullptr};

void copyRegs(CoreContext* s, CoreContext* d) {
  for (int i = 0; i < 30; ++i) d->gpr[i] = s->gpr[i];
  d->elr_el1 = s->elr_el1;
  d->esr_el1 = s->esr_el1;
  d->lr = s->lr;
  d->sprs_el1 = s->sprs_el1;
  d->sp_el0 = s->sp_el0;
}

uint32_t calcNextCore(Task* t) {
  // return (++cc % 4);
  // return Std::hash(SystemTimer::getTimer()->counter_lo) % 4;

  if (t->isPinnedToCore()) return t->getPinnedCore();
  int outCore = 0;
  uint32_t count = 0xfffffff;

  for (int i = 0; i < NUM_CORES; ++i) {
    Core::runningQLock[i]->getLock();

    if (Core::runningQ[i][t->p]->getSize() < count) {
      count = Core::runningQ[i][t->p]->getSize();
      outCore = i;
    }
    Core::runningQLock[i]->release();
  }
  return outCore;
}

void wakeUpTimers() {
  /*
    Waking up sleeping Tasks
    This is dangerous because we're doing memory alloc stuff (remove, insert)
    without any protection from the interrupted Task
  */
  Core::sleepingQLock->getLock();
  Core::sleepingQ->quickSort();

  // Try to make it safe
_begin_wake_up:
  for (auto t : *Core::sleepingQ) {
    if (t->timer != 0 && t->timer <= SystemTimer::getCounter()) {
      t->timer = 0;
      nextCore = calcNextCore(t);
      Core::runningQLock[nextCore]->getLock();
      if (!Core::sleepingQ->removeElement(t))
        Core::panic("Not found in sleeping Q.\n");
      Core::runningQ[nextCore][t->p]->add(t);
      Core::runningQLock[nextCore]->release();
      goto _begin_wake_up;
    }
  }
  Core::sleepingQLock->release();
}

void reschedule(CoreContext* regs) {
  Task* next;
  uint64_t c;

  if (Core::isPreamptable()) {
    Core::runningQLock[get_core()]->getLock();
    for (int i = 0; i < PRIORITIES; ++i) {
      if (Core::runningQ[get_core()][i]->getSize() > 0) {
        c = Std::hash(SystemTimer::getTimer()->counter_lo) %
            Core::runningQ[get_core()][i]->getSize();
        next = *Core::runningQ[get_core()][i]->get(c);
        break;
      }
    }

    Core::runningQLock[get_core()]->release();

    copyRegs(regs, &current->context);
    copyRegs(&next->context, regs);
    current = next;
    if (next->context.elr_el1 < 0xFFFF000000000000) {
      Console::print_no_lock("Returning to wrong address 0x%x\n",
                             next->context.elr_el1);
      _hang_forever();
    }
  }
}

// Current EL with SPx
extern "C" void irq_handler_spx(CoreContext* regs) {
  unsigned int irq_ack_reg = MMIO::read(GICC_IAR);
  unsigned int irq = irq_ack_reg & 0x3FF;

  rpi_sys_timer_t* timer;
  // Console::print_no_lock("@@@@ Begin Int %d @@@@\n", irq);
  switch (irq) {
    case (SYSTEM_TIMER_IRQ_1):

      /* Wake up expired timers */
      wakeUpTimers();

      /* We reschedule */
      reschedule(regs);

      /* We tell other cores to reschedule */
      GIC400::send_sgi(SYSTEM_RESCHEDULE_IRQ, 1);
      GIC400::send_sgi(SYSTEM_RESCHEDULE_IRQ, 2);
      GIC400::send_sgi(SYSTEM_RESCHEDULE_IRQ, 3);

      SystemTimer::WaitMicroT1(2000);  // 2ms
      SystemTimer::getTimer()->control_status |= 0b0010;

      break;

      /* We were told by Core0 to reschedule */
    case SYSTEM_RESCHEDULE_IRQ:
      reschedule(regs);
      break;

    case SYS_TIMER_IRQ_3:
      Console::print_no_lock("Received Timer interrupt 3 on Core%d\n",
                             get_core());
      SystemTimer::getTimer()->control_status |= 0b1000;
      break;

    case SYSTEM_SLEEP_IRQ:

      Core::runningQLock[get_core()]->getLock();
      // Putting current to sleep
      Core::runningQ[get_core()][current->p]->quickSort();
      Core::runningQ[get_core()][current->p]->removeElement(current);

      Core::runningQLock[get_core()]->release();

      Core::sleepingQLock->getLock();
      Core::sleepingQ->add(current);
      Core::sleepingQLock->release();
      reschedule(regs);
      break;

    case (SYSTEM_UARTRX_IRQ):
      car = Console::getKernelConsole()->readChar();
      // Console::print_no_lock("Uart int! %d\n", car);
      /*  if (car == 13)
       {
         y += 18;
         x = 0;
       }
       if (at <= 1)
         at = 0xf;
       if (drawChar(car, x, y, at))
       {
         x += 16;
         if ((x + 16) > 1920)
         {
           y += 18;
           x = 0;
         }
       } */
      Console::print_no_lock("\n\n");
      for (int i = 0; i < NUM_CORES; ++i) {
        Console::print_no_lock("#################\nCore%d\n", i);
        for (int p = 0; p < PRIORITIES; ++p) {
          if (Core::runningQ[i][p]->getSize() > 0)
            Console::print_no_lock("RunninQ[%d] Core%d: %d\n", p, i,
                                   Core::runningQ[i][p]->getSize());
        }
      }
      Console::print_no_lock("SleepingQ: %d\n\n", Core::sleepingQ->getSize());
      Console::print_no_lock("\n\n");
      timer = SystemTimer::getTimer();
      Console::print_no_lock("System Timer Counter: %x\n",
                             SystemTimer::getCounter());
      Console::print_no_lock("System Timer Lo: %x\n", timer->counter_lo);
      Console::print_no_lock("System Timer Hi: %x\n", timer->counter_hi);

      Console::print_no_lock("System Timer Compare0: %x\n", timer->compare0);
      Console::print_no_lock("Allocations: %d\n", allocations);
      Console::print_no_lock("\n\n");
      break;

      /* Halt core */
    case SYSTEM_HALT_IRQ:
      _hang_forever();

      /* Spourius interrupts */
    case SYSTEM_SPOURIOUS_IRQ:
      Console::print_no_lock("SPOURIOUS INT RECEIVED Core%d: %x\r\n",
                             get_core(), irq);
      break;
    default:
      Console::print_no_lock("Unknown Interrupt %d on  Core%d\r\n", irq,
                             get_core());
      break;
  }
// Console::print_no_lock("@@@@ END Int %d @@@@\n", irq);
_done:
  MMIO::write(GICC_EOIR, irq);
}

// Current level w/ SP0
extern "C" void irq_handler_sp0(CoreContext* regs) {
  irq_handler_spx(regs);
  /* Console::print_no_lock("Received IRQ Exception on core %d!!!\n",
  get_core()); Console::print_no_lock("Current Level with SP0\n"); */
  //_hang_forever();
}

void printRegs(CoreContext* regs) {
  for (int i = 0; i < 30; ++i)
    Console::print_no_lock("x%d=%x\n", i, regs->gpr[i]);
  Console::print_no_lock(
      "SPSR=%x\nELR=%x\nESR=%x\nLR=%x\nSP_EL0=%x\nFAR_EL1=%x\n", regs->sprs_el1,
      regs->elr_el1, regs->esr_el1, regs->lr, regs->sp_el0, regs->far_el1);

  Console::print_no_lock("Current PID: %d\n", current->pid);
  Console::print_no_lock("Allocation: %d\n", allocations);
}

extern "C" void sync_handler_sp0(CoreContext* regs) {
  uint32_t i = 0;
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

extern "C" void sync_handler_spx(CoreContext* regs) {
  uint32_t i = 0;
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
