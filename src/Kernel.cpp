#include "include/BootAllocator.h"
#include "include/Console.h"
#include "include/Core.h"
#include "include/GIC.h"
#include "include/Gpio.h"
#include "include/IRQ.h"
#include "include/KernelHeapAllocator.h"
#include "include/Mem.h"
#include "include/Mmu.h"
#include "include/SMP.h"
#include "include/Spinlock.h"
#include "include/Stdlib.h"
#include "include/String.h"
#include "include/SystemTimer.h"
#include "include/Task.h"
#include "include/Uart.h"
#include "include/Vector.h"

extern void *_boot_alloc_start;
extern void *_boot_alloc_end;

extern void *_heap_start;
extern void *_heap_end;

extern "C" void init_core();
extern "C" uint64_t core_activations[4];
extern "C" uint64_t get_sp();
void kernelThread() {
  // Core::enableIRQ();
  uint32_t core = get_core();
  uint64_t pid = Core::current[core]->pid;
  uint64_t count = 0;
  while (true) {
    // c++;
    //  if (core != 0) core_activations[core]++;
    uint64_t sp = get_sp();
    Core::preemptDisable();
    Console::print(
        "Kernel thread #%d on Core%d PID: %d!\nSP: %x Free: %d Count: %d\n",
        Core::current[core]->pid, core, pid, sp,
        GlobalKernelAlloc::getAllocator()->freeSpace(), count++);

    // Core::spinms(1000 + 10 * core);
    Core::preemptEnable();
    Core::spinms(100 + 10 * core);
    /*  Console::print("Core%d Running Tasks:\n", core);
     for (Task *t : *Core::runningQ[core]) {
       Console::print("%d,", t->pid);
     }
     Console::print("\n"); */
    /* Console::print("C0: %u C1:%u C2:%u C3:%u\n", core_activations[0],
                   core_activations[1], core_activations[2],
                   core_activations[3]);
    Console::print("Core%d Running Tasks:\n", core);
    for (Task *t : *Core::runningQ[core]) {
      Console::print("%d,", t->pid);
    } */
    // Console::print("\n");

    /*  if (c < 3 && core_activations[get_core()] < 20) {
       Task *t = Task::createKernelTask((uint64_t)&kernelThread);
       disable_irq();
       Core::runningQ[get_core()]->push_back(t);
       enable_irq();
       Console::print("Adding new Task %d\n", t->pid);
     } */

    /* uint64_t c = SystemTimer::getTimer()->counter_lo % 100;
    Vector<Task *> vnext = *Core::runningQ[core];
    Console::print("Switching to %d\n", vnext[c]->pid);
    Core::preemptDisable();
    Core::switchTo(vnext[c]);
    Core::preemptEnable(); */
  }
  _hang_forever();
}

extern "C" void kernel_main() {
  Core::disableIRQ();

  BootAllocator boot_allocator = BootAllocator(
      (unsigned char *)&_boot_alloc_start, (unsigned char *)&_boot_alloc_end);

  GlobalKernelAlloc::setAllocator(&boot_allocator);
  // We can use new with the boot allocator
  DriverManager::init();
  GPIO *gpio = new GPIO();
  UART *uart = new UART(gpio);
  GIC400 *gic = new GIC400();
  SystemTimer *timer = new SystemTimer();
  DriverManager::load(gpio);
  DriverManager::load(uart);
  DriverManager::load(gic);
  DriverManager::load(timer);

  DriverManager::startAll();

  Console::setKernelConsole(uart);
  Console::print("\n\n\n\n\n\n############################################\n");
  Console::print("Current EL: %u\n", Std::getCurrentEL());

  Console::print("BootAlloc Start: 0x%x BootAlloc end: 0x%x\n",
                 &_boot_alloc_start, &_boot_alloc_end);
  Console::print("Heap Start: 0x%x Heap end: 0x%x\n",
                 (unsigned char *)&_heap_start, (unsigned char *)&_heap_end);

  KernelHeapAllocator *kha = new KernelHeapAllocator(
      (unsigned char *)&_heap_start, (unsigned char *)&_heap_end);
  GlobalKernelAlloc::setAllocator(kha);
  Console::print("List of loaded drivers:\n");
  for (auto d : *DriverManager::getAll())
    Console::print("Driver %s\n", d->getName());

  initSchedLock();

  Console::print("Timer init on core: %d\n", get_core());
  Console::print("############################################\n");

  // Core::runningQ[get_core()] = new Vector<Task *>();

  /*  Task *t = Task::createKernelTask((uint64_t)&kernelThread);
   Core::runningQ[get_core()]->push_back(t); */
  // Core::switchTo(t);

  Task *n;
  for (int i = 0; i < THREAD_N; ++i) {
    Task *t = Task::createKernelTask((uint64_t)&kernelThread);
    Core::runningQ[get_core()][i] = t;
    if (i == 0) n = t;
    // Core::current[get_core()] = t;
  }

  Core::spinms(3000);
  Core::start(1, &init_core);
  Core::spinms(3000);
  Core::start(2, &init_core);
  Core::spinms(3000);
  Core::start(3, &init_core);
  Core::spinms(5000);

  Core::current[get_core()] = new Task();

  SystemTimer::WaitMicroT1(200000);
  // SystemTimer::WaitMicroT3(300000);
  Core::enableIRQ();
  /*  Core::enableIRQ();
   Core::preemptDisable();
   Core::switchTo(n);
   Core::preemptEnable(); */

  _hang_forever();
}
