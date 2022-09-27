#ifndef TASK_H
#define TASK_H
#include <stdint.h>
#include "IRQ.h"


class Task {
 public:
  Task();
  CoreContext context;
  core_context second;
  long state;
  uint64_t pid;
  long priority;
  long ticks;
  uint64_t c;
  uint64_t flags;
  static Task *createKernelTask(uint64_t entryPoint);
  static uint64_t freePID;
};

void idleTask();
void kernelTask();

#endif