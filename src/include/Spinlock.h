#ifndef LOCK_H
#define LOCK_H

#include <stdint.h>

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
  void getLock();
  void release();
  splck_t l;

 private:
  uint64_t ownerPid;
};

#endif