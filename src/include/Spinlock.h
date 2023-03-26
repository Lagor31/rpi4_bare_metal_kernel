#ifndef LOCK_H
#define LOCK_H

#include <stdint.h>

#include "List.h"

#define SPLCK_UNLOCKED (0)
#define SPLCK_LOCKED (1)

#define SPLCK_INITIALIZER \
  { .val = SPLCK_UNLOCKED }
struct splck {
  unsigned int val;
};
typedef struct splck splck_t;

class Spinlock {
 public:
  Spinlock();
  void spin();
  void release();
  void get();
  void free();

  splck_t l;
  uint64_t ownerPid;

 private:
  uint32_t testAndSet();
};

#endif