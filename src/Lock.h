#ifndef LOCK_H
#define LOCK_H

/* A simple spinlock. Safe in any context */
struct splck {
   unsigned int val;
};

#define SPLCK_UNLOCKED (0)
#define SPLCK_LOCKED (1)

#define SPLCK_INITIALIZER \
  { .val = SPLCK_UNLOCKED }

typedef struct splck splck_t;

void splck_init(splck_t *lck);

void splck_lck(splck_t *lck);

void splck_done(splck_t *lck);

#endif