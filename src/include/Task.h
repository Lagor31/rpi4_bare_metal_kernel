#ifndef TASK_H
#define TASK_H
#include <stdint.h>

#include "IRQ.h"

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
  void sleep(uint64_t);
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