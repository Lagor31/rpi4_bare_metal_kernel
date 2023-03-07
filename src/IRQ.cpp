#include "include/IRQ.h"

#include <stddef.h>
#include <stdint.h>

#include "include/Console.h"
#include "include/Core.h"
#include "include/GIC.h"
#include "include/KernelHeapAllocator.h"
#include "include/Lists/ArrayList.hpp"
#include "include/Mem.h"
#include "include/Spinlock.h"
#include "include/Stdlib.h"
#include "include/SystemTimer.h"
#include "include/Vector.h"
#include "include/fb.h"
#include "include/io.h"
#include "include/sysregs.h"

using SD::Lists::ArrayList;

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

uint32_t calcNextCore() {
  // return (++cc % 4);
  // return Std::hash(SystemTimer::getTimer()->counter_lo) % 4;

  int outCore = 0;
  uint32_t count = 0xfffffff;

  for (int i = 0; i < 4; ++i) {
    Core::runningQLock[i]->getLock();

    if (Core::runningQ[i]->count() <= count) {
      count = Core::runningQ[i]->count();
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

  for (int i = 0; i < Core::sleepingQ->count(); ++i) {
    uint64_t cTimer = Core::sleepingQ->get(i)->timer;
    if (cTimer != 0 && cTimer <= SystemTimer::getTimer()->counter_lo) {
      Task* t = Core::sleepingQ->get(i);
      t->timer = 0;
      // uint32_t nextCore = get_core();
      // while ((++nextCore % 4) == get_core());
      nextCore = calcNextCore();
      // Console::print_no_lock("PID %d to Core%d\n", t->pid, nextCore);
      Core::runningQLock[nextCore]->getLock();
      Core::runningQ[nextCore]->insert(t);
      Core::sleepingQ->remove(i);
      Core::runningQLock[nextCore]->release();
      goto _end_sleep;
    }
  }
_end_sleep:
  Core::sleepingQLock->release();
}

void reschedule(CoreContext* regs) {
  Task* next;
  uint64_t c;

  if (Core::isPreamptable()) {
    Core::runningQLock[get_core()]->getLock();

    c = Std::hash(SystemTimer::getTimer()->counter_lo) %
        Core::runningQ[get_core()]->count();
    next = Core::runningQ[get_core()]->get(c);
    Core::runningQLock[get_core()]->release();

    copyRegs(regs, &Core::current[get_core()]->context);
    copyRegs(&next->context, regs);
    Core::current[get_core()] = next;
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
  unsigned int cpu = (irq_ack_reg >> 10) & 7;

  Task* next;
  uint64_t c;
  rpi_sys_timer_t* timer;

  uint32_t index = 0;
  uint32_t remCount = 0;

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

    case SYSTEM_SLEEP_IRQ:

      Core::runningQLock[get_core()]->getLock();
      // Putting current to sleep
      for (int i = 0; i < Core::runningQ[get_core()]->count(); ++i) {
        if (Core::current[get_core()]->pid ==
            Core::runningQ[get_core()]->get(i)->pid) {
          Core::runningQ[get_core()]->remove(i);
          break;
        }
      }

      Core::runningQLock[get_core()]->release();

      Core::sleepingQLock->getLock();
      Core::sleepingQ->insert(Core::current[get_core()]);
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
      for (int i = 0; i < 4; ++i) {
        Console::print_no_lock("#################\nCore%d\n", i);
        Console::print_no_lock("RunninQ Core%d: %d\n", i,
                               Core::runningQ[i]->count());
      }
      Console::print_no_lock("SleepingQ: %d\n\n", Core::sleepingQ->count());
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

  Console::print_no_lock("Current PID: %d\n", Core::current[get_core()]->pid);
}

extern "C" void sync_handler_sp0(CoreContext* regs) {
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

extern "C" void sync_handler_spx(CoreContext* regs) {
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
