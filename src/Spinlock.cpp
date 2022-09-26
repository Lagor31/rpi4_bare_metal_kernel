#include "include/Spinlock.h"

#include "include/Console.h"
#include "include/Core.h"
#include "include/GIC.h"

extern "C" void spin_lock(volatile unsigned int *t);
extern "C" void spin_unlock(volatile unsigned int *t);

Spinlock::Spinlock() { this->l.val = SPLCK_UNLOCKED; }

void Spinlock::getLock() {
  spin_lock(&this->l.val);
  ownerPid = Core::current[get_core()]->pid;
}

void Spinlock::release() {
  spin_unlock(&this->l.val);
  ownerPid = 0;
}
