#include "include/Spinlock.h"

#include "include/Console.h"
#include "include/GIC.h"

Spinlock::Spinlock() { this->l.val = SPLCK_UNLOCKED; }

extern "C" void spin_lock(volatile unsigned int *t);
extern "C" void spin_unlock(volatile unsigned int *t);

void Spinlock::getLock() { spin_lock(&this->l.val); }

void Spinlock::release() { spin_unlock(&this->l.val); }
