#include "include/SMP.h"

#include <stdint.h>

#include "include/Console.h"
#include "include/Core.h"
#include "include/GIC.h"
#include "include/IRQ.h"
#include "include/Lists/ArrayList.hpp"
#include "include/Stdlib.h"
using SD::Lists::SinglyLinkedList;

extern "C" void _wait_for_interrupt();
extern void kernelThread();
extern "C" void initSecondaryCore() {
  Core::disableIRQ();
  uint64_t core = get_core();

  // Core::runningQ[core] = new Vector<Task *>();
  //  if (core != 1) _wait_for_interrupt();
  Core::runningQ[get_core()] = new SinglyLinkedList<Task*>();

  // if (core == 3) {
  Task* idle = Task::createKernelTask((uint64_t)&idleTask);
  Core::runningQ[core]->insert(idle);

  for (int i = 0; i < THREAD_N; ++i) {
    Task* t = Task::createKernelTask((uint64_t)&kernelTask);
    Core::runningQ[core]->insert(t);
  }

  // if (core == 3) {
  Task* screen = Task::createKernelTask((uint64_t)&screenTask);
  Core::runningQ[core]->insert(screen);
  //}
  Core::current[core] = new Task();
  Console::print("@@@@@@@@@@@@@@@\n\n Core %d active!\n\n@@@@@@@@@@@@@@@\n",
                 core);

  Core::enableIRQ();
  // Core::switchTo(idle);
  /*   Core::preemptDisable();
    Core::switchTo(n);
    Core::preemptEnable();
   */  //}

  _wait_for_interrupt();
}