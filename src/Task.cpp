#include "include/Task.h"

#include <stdint.h>

#include "include/Console.h"
#include "include/Core.h"
#include "include/GIC.h"
#include "include/Mem.h"
#include "include/Stdlib.h"
#include "include/SystemTimer.h"
#include "include/buddy_alloc.h"
#include "include/fb.h"
#include "include/io.h"
#include "include/mb.h"

extern "C" uint64_t core_activations[4];
extern "C" uint64_t get_sp();
extern "C" unsigned int get_core();

void idleTask() { _hang_forever(); }

void topBarTask() {
  uint64_t count = 0;
  uint8_t at = 0xF;
  uint8_t tAt = 0xF;
  while (true) {
    uint32_t core = get_core();
    uint64_t pid = Core::current[core]->pid;
    switch (core) {
      case 0:
        tAt = 0xFF;
        break;
      case 1:
        tAt = 0xCC;
        break;
      case 2:
        tAt = 0xBB;
        break;
      case 3:
        tAt = 0xDD;
        break;
      default:
        Core::panic("Wrong CPU!!!!\n");
    }

    /*  do {
       tAt = Std::hash(SystemTimer::getCounter()) % 16;
     } while (tAt == at); */
    // Console::print("TopBAR form PID=%d On Core=%d!\n", pid, core);

    at = tAt;
    uint64_t sp = get_sp();
    char* hText = "FedeFede";
    Core::disableIRQ();
    for (int i = 0; i < 8; ++i) {
      drawChar(hText[i], i * 16 + 800, 0, at);
    }
    Core::enableIRQ();
    Core::current[core]->sleep(1000);
  }
  _hang_forever();
}

void screenTask() {
  Circle* drawMe;
  int y = 0;
  bool dir = 0;
  uint32_t c = 0;
  while (true) {
    /* Console::print("Screen Task PID=%d Core=%d\n",
                   Core::current[get_core()]->pid, get_core()); */
    fb_lock->getLock();
    if (circles->count() > 0) {
      drawMe = circles->get(0);

      Core::disableIRQ();
      circles->remove(0);
      Core::enableIRQ();

      Core::preemptDisable();
      drawCircle(drawMe->x, drawMe->y, drawMe->radius, drawMe->attr,
                 drawMe->fill);
      Core::disableIRQ();
      delete drawMe;
      Core::enableIRQ();
      Core::preemptEnable();
    }

    fb_lock->release();

    /*
        ++c;
        !dir ? ++y : --y;

        if (c > 0 && c % 200 == 0) dir = !dir;
        if (c > 1000) {
          dir = !dir;
          c = 0;
          y = 0;
        }
        mbox[0] = 7 * 4;  // Length of message in bytes
        mbox[1] = MBOX_REQUEST;

        mbox[2] = MBOX_TAG_SETVIRTOFF;
        mbox[3] = 8;
        mbox[4] = 8;
        mbox[5] = 0;  // Value(x)
        mbox[6] = y;  // Value(y)

        mbox[7] = MBOX_TAG_LAST;
        if (get_core() == 0) mbox_call(MBOX_CH_PROP);
     */
    Core::current[get_core()]->sleep(10);
  }
}
void kernelTask() {
  uint64_t count = 0;
  while (true) {
    uint32_t core = get_core();
    uint64_t pid = Core::current[core]->pid;
    uint64_t sp = get_sp();
    // Core::preemptDisable();
    Core::current[core]->sleep(2000 +
                               Std::hash(SystemTimer::getCounter()) % 1000);
    /* Console::print(
        "\nKernel thread #%d on Core%d PID: %d!\nSP: %x Free: %d Count: %d\n",
        Core::current[core]->pid, core, pid, sp,
       GlobalKernelAlloc::freeSpace(), count++); */
    // for (int i = 0; i < 100; ++i) Std::hash(i);
    uint32_t x = Std::hash(SystemTimer::getCounter()) % (1920);
    uint32_t y = Std::hash(SystemTimer::getCounter()) % (1080) + 32;

    uint32_t radius = Std::hash(SystemTimer::getCounter()) % 10 + 3;
    uint8_t attr = Std::hash(SystemTimer::getCounter()) % 255 + 16;
    uint32_t filled = Std::hash(SystemTimer::getCounter()) % 2;
    Core::disableIRQ();
    Circle* paintMe = (Circle*)GlobalKernelAlloc::alloc(sizeof(Circle));
    Core::enableIRQ();

    paintMe->x = x;
    paintMe->y = y;

    switch (core) {
      case 0:
        paintMe->attr = 0xFF;
        break;
      case 1:
        paintMe->attr = 0xCC;
        break;
      case 2:
        paintMe->attr = 0xBB;
        break;
      case 3:
        paintMe->attr = 0xDD;
        break;
      default:
        Core::panic("Wrong CPU!!!!\n");
    }
    paintMe->fill = 1;
    paintMe->radius = radius;
    // Console::print("Printing Circle form PID=%d On Core=%d!\n", pid, core);
    paintCircle(paintMe);
  }
  _hang_forever();
}

void Task::sleep(uint32_t ms) {
  Task* goingToSleep = nullptr;
  rpi_sys_timer_t* timer;

  Task* curr = Core::current[get_core()];

  timer = SystemTimer::getTimer();
  uint32_t lo;
  uint32_t hi;

  do {
    lo = timer->counter_lo;
    hi = timer->counter_hi;
  } while (hi != timer->counter_hi);
  curr->timer = lo + ms * (uint32_t)1000;
  GIC400::send_sgi(SYSTEM_SLEEP_IRQ, get_core());
}

uint64_t Task::freePID = 1;

Task::Task() { c = 0; }

Task* Task::createKernelTask(uint64_t entryPoint) {
  Task* out = new Task();
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
  for (int i = 0; i < 30; ++i) out->context.gpr[i] = 0;

  out->context.gpr[10] = Task::freePID;

  out->c = 0;
  out->timer = 0;
  return out;
}
