#ifndef LOCK_H
#define LOCK_H

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

 private:
  splck_t l;
};

#endif