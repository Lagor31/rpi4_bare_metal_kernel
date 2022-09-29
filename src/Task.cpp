#include "include/Task.h"

#include <stdint.h>

#include "include/Console.h"
#include "include/Core.h"
#include "include/GIC.h"
#include "include/Mem.h"
#include "include/Stdlib.h"
#include "include/SystemTimer.h"

extern "C" uint64_t core_activations[4];
extern "C" uint64_t get_sp();
extern "C" unsigned int get_core();

void idleTask() { _hang_forever(); }

void kernelTask() {
  uint32_t core = get_core();
  uint64_t pid = Core::current[core]->pid;
  uint64_t count = 0;
  while (true) {
    uint64_t sp = get_sp();
    Core::preemptDisable();
    Console::print(
        "\nKernel thread #%d on Core%d PID: %d!\nSP: %x Free: %d Count: %d\n",
        //"Switching to: %d\n",
        Core::current[core]->pid, core, pid, sp, GlobalKernelAlloc::freeSpace(),
        count++);

    Core::preemptEnable();
    for (int i = 0; i < 100; ++i) Std::hash(i);
    GIC400::send_sgi(3, get_core());
  }
  _hang_forever();
}

void Task::sleep(uint32_t ms) {
  Core::preemptDisable();
  /*
    Console::print("Task Running List:\n");
    Core::printList(Core::runningQ[get_core()]);

    Console::print("Task Sleeping List:\n");
    Core::printList(Core::sleepingQ[get_core()]);
   */
  // Core::scheduler->getLock();
  uint32_t core = get_core();
  for (int i = 0; i < Core::runningQ[core]->count(); ++i) {
    if (this->pid == Core::runningQ[core]->get(i)->pid) {
      Core::runningQ[core]->remove(i);
      this->timer = SystemTimer::getCounter() + (ms * 1000);
      Core::sleepingQ[core]->insert(this);
      break;
    }
  }
  uint64_t c0 = SystemTimer::getTimer()->compare0;
  uint32_t h = Std::hash(c0);
  uint32_t num = h % Core::runningQ[core]->count();
  Task *next = Core::runningQ[get_core()]->get(num);
  // Core::scheduler->release();
  /* Console::print("Task Running List After:\n");
  Core::printList(Core::runningQ[get_core()]);

  Console::print("Task IRQ Sleeping List After:\n");
  Core::printList(Core::sleepingQ[get_core()]); */
  // Core::preemptDisable();
  Core::switchTo(next);
  Core::preemptEnable();
  // Core::preemptEnable();
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
  out->timer = 0;
  return out;
}
