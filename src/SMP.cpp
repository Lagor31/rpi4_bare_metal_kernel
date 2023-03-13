#include "include/SMP.h"

#include <stdint.h>

#include "include/Console.h"
#include "include/Core.h"
#include "include/GIC.h"
#include "include/IRQ.h"
#include "include/List.h"
#include "include/Stdlib.h"

extern "C" void _wait_for_interrupt();
extern void kernelThread();
extern "C" void initSecondaryCore() {
  Core::disableIRQ();
  uint64_t core = get_core();

  // Core::runningQ[core] = new Vector<Task *>();
  //  if (core != 1) _wait_for_interrupt();
  for (int p = 0; p < PRIORITIES; ++p)
    Core::runningQ[get_core()][p] = new ArrayList<Task*>();

  // if (core == 3) {
  Task* idle = Task::createKernelTask((uint64_t)&idleTask);
  Core::runningQ[core][idle->p]->add(idle);

  for (int i = 0; i < THREAD_N; ++i) {
    Task* t = Task::createKernelTask((uint64_t)&kernelTask);
    t->p = 19;
    Core::runningQ[core][t->p]->add(t);
  }

  // if (core == 3) {
  Task* screen = Task::createKernelTask((uint64_t)&screenTask);
  screen->p = 0;
  screen->pinToCore(get_core());
  Core::runningQ[core][screen->p]->add(screen);
  // }
  current = new Task();
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