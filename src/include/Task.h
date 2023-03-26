#ifndef TASK_H
#define TASK_H
#include <stdint.h>
#include "Spinlock.h"

//#include "IRQ.h"

struct CoreContext {
  uint64_t gpr[30];
  /// The link register, aka x30.
  uint64_t lr;
  uint64_t elr_el1;
  uint64_t sprs_el1;
  uint64_t esr_el1;
  uint64_t sp_el0;
  uint64_t far_el1;
} __attribute__((packed));

class Task {
 public:
  Task();
  CoreContext context;
  long state;
  uint64_t pid;
  long priority;
  long ticks;
  uint32_t p;
  uint64_t premption;
  uint64_t flags;
  static Task *createKernelTask(uint64_t entryPoint);
  static uint64_t freePID;
  uint64_t timer;
  Spinlock *sleepingOnLock;
  void sleep(uint64_t);
  void sleep();
  void pinToCore(uint32_t core);
  bool isPinnedToCore();
  uint32_t getPinnedCore();
  void disablePinning();
 private:
  bool isCorePinned;
  uint32_t corePinned;
};

void idleTask();
void kernelTask();
void topBarTask();
void screenTask();


#endif