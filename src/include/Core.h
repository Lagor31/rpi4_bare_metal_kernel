#ifndef CORE_H
#define CORE_H

#include <stddef.h>
#include <stdint.h>

#include "Lists/SinglyLinkedList.hpp"
#include "Task.h"
using SD::Lists::SinglyLinkedList;

#define THREAD_N 5

class Core {
 public:
  static void spinms(uint32_t);
  static void start(uint32_t core, void (*func)(void));
  static void disableIRQ();
  static void enableIRQ();
  static void panic(const char *s);
  static Task *current[4];
  static void printList(SinglyLinkedList<Task *> *l);

  static Spinlock *scheduler[4];
  // static Vector<Task*> *runningQ[4];
  // static Task *runningQ[4][THREAD_N];
  static SinglyLinkedList<Task *> *runningQ[4];
  static SinglyLinkedList<Task *> *sleepingQ[4];
  static void preemptDisable();
  static void preemptEnable();
  static bool isPreamptable();
};

#endif