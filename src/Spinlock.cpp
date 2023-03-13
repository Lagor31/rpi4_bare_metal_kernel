#include "include/Spinlock.h"

#include "include/Console.h"
#include "include/Core.h"
#include "include/GIC.h"

extern "C" void _spin_lock(volatile unsigned int *t);
extern "C" void _spin_unlock(volatile unsigned int *t);

Spinlock::Spinlock() { this->l.val = SPLCK_UNLOCKED; }

void Spinlock::getLock() {
  _spin_lock(&this->l.val);
  ownerPid = current->pid;
}

void Spinlock::release() {
  _spin_unlock(&this->l.val);
  ownerPid = 0;
}
