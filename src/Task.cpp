#include "include/Task.h"

#include <stdint.h>

#include "include/BuddyAlloc.h"
#include "include/Console.h"
#include "include/Core.h"
#include "include/FrameBuffer.h"
#include "include/GIC.h"
#include "include/IO.h"
#include "include/Mailbox.h"
#include "include/Mem.h"
#include "include/Pstate.h"
#include "include/Stdlib.h"
#include "include/SystemTimer.h"

extern "C" uint64_t core_activations[NUM_CORES];
extern "C" uint64_t get_sp();
extern "C" unsigned int get_core();

void idleTask() { _hang_forever(); }

void topBarTask() {
  while (true) {
    current->sleep(1000);
  }
  _hang_forever();
}

void screenTask() {
  Circle* drawMe;
  while (true) {
    /*  Console::print("Screen Task PID=%d Core=%d\n",
                    current->pid, get_core()); */
  _begin:

    fb_lock->get();
    if (circles->getSize() > 0) {
      drawMe = *circles->get(0);

      Core::disableIRQ();
      circles->remove(0);
      Core::enableIRQ();

      Core::preemptDisable();
      drawCircle(drawMe->x, drawMe->y, drawMe->radius, getCoreColor(get_core()),
                 drawMe->fill);
      Core::disableIRQ();
      delete drawMe;
      Core::enableIRQ();
      Core::preemptEnable();
      fb_lock->free();

    } else {
      fb_lock->free();
      goto _sleep;
    }

    goto _begin;
  _sleep:
    current->sleep(16);
  }
}
void kernelTask() {
  while (true) {
    uint32_t core = get_core();
    current->sleep(2000 + Std::hash(SystemTimer::getCounter()) % 1000);

    uint32_t x = Std::hash(SystemTimer::getCounter()) % (1920);
    uint32_t y = Std::hash(SystemTimer::getCounter()) % (1080) + 32;

    uint32_t radius = Std::hash(SystemTimer::getCounter()) % 10 + 3;
    Core::disableIRQ();
    Circle* paintMe = (Circle*)GlobalKernelAlloc::alloc(sizeof(Circle));
    Core::enableIRQ();
    paintMe->x = x;
    paintMe->y = y;
    paintMe->attr = getCoreColor(core);
    paintMe->fill = 1;
    paintMe->radius = radius;
    // Console::print("Printing Circle form PID=%d On Core=%d!\n", pid, core);
    /*  uint32_t pstate = read_pstate();
     Console::print("PSTATE: 0x%x\n", pstate); */
    paintCircle(paintMe);
    current->sleep(1000);
  }
  _hang_forever();
}

void Task::sleep(uint64_t ms) {
  Task* curr = current;
  curr->timer = SystemTimer::getCounter() + ms * (uint64_t)1000;
  sleep();
}

uint64_t Task::freePID = 1;
extern void reschedule(CoreContext* regs);

void Task::sleep() { GIC400::send_sgi(SYSTEM_SLEEP_IRQ, get_core()); }

Task::Task() { premption = 0; }

bool Task::isPinnedToCore() { return this->isCorePinned; }
void Task::pinToCore(uint32_t core) {
  if (core > (NUM_CORES - 1)) return;
  this->isCorePinned = true;
  this->corePinned = core;
}
uint32_t Task::getPinnedCore() {
  if (!isCorePinned) Core::panic("Haven't checked if I'm core pinned!!!");
  return this->corePinned;
}

void Task::disablePinning() { this->isCorePinned = false; }

Task* Task::createKernelTask(uint64_t entryPoint) {
  Task* out = new Task();
  out->context.lr = entryPoint;
  out->context.elr_el1 = entryPoint;
  uint64_t stack = (uint64_t)GlobalKernelAlloc::alloc(4096);
  stack += 4096;
  out->context.sp_el0 = stack;
  // 0x364 int enabled
  out->context.sprs_el1 = 0x364;
  out->pid = Task::freePID++;
  for (int i = 0; i < 30; ++i) out->context.gpr[i] = 0;
  out->isCorePinned = false;
  out->corePinned = 0;
  out->context.gpr[10] = Task::freePID;
  out->p = 20;
  out->premption = 0;
  out->timer = 0;
  return out;
}
