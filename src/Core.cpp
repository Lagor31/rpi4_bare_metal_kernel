#include "include/Core.h"

#include "include/Console.h"
#include "include/GIC.h"
#include "include/Lists/ArrayList.hpp"
#include "include/Stdlib.h"
#include "include/SystemTimer.h"

using SD::Lists::ArrayList;

extern "C" void enable_irq(void);
extern "C" void disable_irq(void);

extern "C" uint64_t get_far_el1();
extern "C" uint64_t get_esr_el1();
extern "C" uint64_t get_elr_el1();

void Core::disableIRQ() { disable_irq(); }
void Core::enableIRQ() { enable_irq(); }

Spinlock* Core::runningQLock[4];
Spinlock* Core::sleepingQLock;

Task* Core::current[4];
SinglyLinkedList<Task*>* Core::runningQ[4];
SinglyLinkedList<Task*>* Core::sleepingQ;

void Core::printList(SinglyLinkedList<Task*>* l) {
  for (uint32_t i = 0; i < l->count(); ++i) {
    Console::print("PID: %d\n", l->get(i)->pid);
  }
}
void Core::preemptDisable() { Core::current[get_core()]->c++; }
void Core::preemptEnable() { Core::current[get_core()]->c--; }
bool Core::isPreamptable() { return Core::current[get_core()]->c <= 0; }

void Core::start(uint32_t core, void (*func)(void)) {
  if (core < 1 || core > 3) return;

  switch (core) {
  case 1:
    store64((unsigned long)0xffff0000000000E0, (unsigned long)func);
    asm volatile("dc civac, %0" : : "r"(0xffff0000000000E0) : "memory");
    asm volatile("sev");
    break;
  case 2:
    store64((unsigned long)0xffff0000000000E8, (unsigned long)func);
    asm volatile("dc civac, %0" : : "r"(0xffff0000000000E8) : "memory");
    asm volatile("sev");
    break;
  case 3:
    store64((unsigned long)0xffff0000000000F0, (unsigned long)func);
    asm volatile("dc civac, %0" : : "r"(0xffff0000000000F0) : "memory");
    asm volatile("sev");
    break;
  }
}

void Core::panic(const char* message) {
  uint32_t i = 0;
  for (; i < 4; ++i) {
    if (get_core() != i) GIC400::send_sgi(1, i);
  }
  i = 0;
  while (i++ < 5) {
    Console::print_no_lock("Panicking on core %d!!!\n", get_core());
    Console::print_no_lock("Message:\n%s\n", message);
    // printRegs(regs);
    Core::spinms(1000 + get_core() * 10);
  }
  _hang_forever();
  // regs->elr_el1 += 4;
  i++;
}

void Core::spinms(uint32_t n) {
  uint64_t counter = SystemTimer::getCounter();
  uint64_t target = (counter + (n * 1000));

  while (counter < target) {
    counter = SystemTimer::getCounter();
  }
}
