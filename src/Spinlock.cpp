#include "include/Spinlock.h"

#include "include/Console.h"
#include "include/Core.h"
#include "include/GIC.h"
#include "include/IRQ.h"
#include "include/List.h"

extern "C" void _spin_lock(volatile unsigned int *t);
extern "C" void _spin_unlock(volatile unsigned int *t);
extern "C" uint64_t _test_set_lock(volatile unsigned int *t);

Spinlock::Spinlock() { this->l.val = SPLCK_UNLOCKED; }

void Spinlock::spin() {
  _spin_lock(&this->l.val);
  ownerPid = current->pid;
}
/*
This is actually a mutex
*/
void Spinlock::get() {
  while (!testAndSet()) {
    current->sleepingOnLock = this;
    /* Console::print_no_lock("PID %d - Lock taken, going to sleep!\n",
                           current->pid); */
    current->sleep();
  }

  ownerPid = current->pid;
}

uint32_t Spinlock::testAndSet() { return _test_set_lock(&this->l.val); }

void Spinlock::free() {
  release();
  GIC400::send_sgi(SYSTEM_WAKEUP_ALL, get_core());
}

void Spinlock::release() {
  ownerPid = 0;
  _spin_unlock(&this->l.val);
}
