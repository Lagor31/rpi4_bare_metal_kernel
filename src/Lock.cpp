#include "Lock.h"

#include "Console.h"

using ltl::console::Console;

void splck_init(splck_t *lck) { lck->val = SPLCK_UNLOCKED; }

void splck_lck(splck_t *lck) {
  while (__atomic_exchange_n(&lck->val, SPLCK_LOCKED, __ATOMIC_ACQUIRE) ==
         SPLCK_LOCKED) {
    //Console::print_no_lock("Locked :(\n");
  }
}

void splck_done(splck_t *lck) {
  __atomic_store_n(&lck->val, SPLCK_UNLOCKED, __ATOMIC_RELEASE);
}