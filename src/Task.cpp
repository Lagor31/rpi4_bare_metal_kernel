#include "include/Task.h"

#include <stdint.h>

#include "include/Mem.h"
uint64_t Task::freePID = 1;

Task::Task() { c = 0; }

Task *Task::createKernelTask(uint64_t entryPoint) {
  Task *out = new Task();
  out->context.lr = entryPoint;
  out->context.elr_el1 = entryPoint;
  uint64_t stack = (uint64_t)GlobalKernelAlloc::alloc(4096);
  stack += 4096;
  out->context.sp_el0 = stack;
  // 0x364 int enabled
  out->context.sprs_el1 = 0x364;
  out->pid = Task::freePID++;
  out->context.gpr[10] = Task::freePID;
  out->c = 0;
  return out;
}
