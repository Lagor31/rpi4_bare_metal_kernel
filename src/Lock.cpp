#include "include/Lock.h"

#include "include/Console.h"
#include "include/GIC.h"

using ltl::console::Console;

void splck_init(splck_t *lck) { lck->val = SPLCK_UNLOCKED; }

extern "C" void spin_lock(volatile unsigned int *t);
extern "C" void spin_unlock(volatile unsigned int *t);

static inline void memory_barrier() { __sync_synchronize(); }


void splck_lck(splck_t *lck) {
  spin_lock(&lck->val);
  //memory_barrier();

  /*  while (__atomic_exchange_n(&lck->val, SPLCK_LOCKED, __ATOMIC_ACQUIRE) ==
          SPLCK_LOCKED) {
     // Console::print_no_lock("Locked on core%d :(\n", get_core());
   }
   // Console::print_no_lock("Freed!\n"); */
}

void splck_done(splck_t *lck) {
  spin_unlock(&lck->val);
  //memory_barrier();

  //__atomic_store_n(&lck->val, SPLCK_UNLOCKED, __ATOMIC_RELEASE);
}
