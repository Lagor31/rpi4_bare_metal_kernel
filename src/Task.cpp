#include "include/Task.h"

#include <stdint.h>

#include "include/Console.h"
#include "include/Core.h"
#include "include/Mem.h"
#include "include/Stdlib.h"
#include "include/SystemTimer.h"

extern "C" uint64_t core_activations[4];
extern "C" uint64_t get_sp();
extern "C" unsigned int get_core();

void idleTask() { _hang_forever(); }

void kernelTask() {
  // Core::enableIRQ();
  uint32_t core = get_core();
  uint64_t pid = Core::current[core]->pid;
  uint64_t count = 0;
  while (true) {
    // c++;
    //  if (core != 0) core_activations[core]++;
    uint64_t sp = get_sp();
    Core::preemptDisable();
    uint32_t h = Std::djb33_hash(SystemTimer::getCounter()) % THREAD_N;

    Console::print(
        "Kernel thread #%d on Core%d PID: %d!\nSP: %x Free: %d Count: %d\n"
        "Switching to: %d\n",
        Core::current[core]->pid, core, pid, sp,
        GlobalKernelAlloc::getAllocator()->freeSpace(), count++, h);

    // Core::spinms(1000 + 10 * core);
    Task *next = Core::runningQ[get_core()][h];
    Core::preemptEnable();
    Core::spinms(100 + 10 * core);
    for (int i = 0; i < 100; ++i) Std::djb33_hash(i);
    // sched_lock->getLock();
    // sched_lock->release();
    Core::switchTo(next);
  }
  _hang_forever();
}

uint64_t Task::freePID = 1;

Task::Task() { c = 0; }

Task *Task::createKernelTask(uint64_t entryPoint) {
  Task *out = new Task();
  out->context.lr = entryPoint;
  out->second.lr = entryPoint;
  out->context.elr_el1 = entryPoint;
  uint64_t stack = (uint64_t)GlobalKernelAlloc::alloc(4096);
  stack += 4096;
  out->context.sp_el0 = stack;
  out->second.sp = stack;
  // 0x364 int enabled
  out->context.sprs_el1 = 0x364;
  out->pid = Task::freePID++;
  out->context.gpr[10] = Task::freePID;
  out->c = 0;
  return out;
}
