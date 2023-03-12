#ifndef CORE_H
#define CORE_H

#include <stddef.h>
#include <stdint.h>

#include "List.h"
#include "Task.h"

#define THREAD_N 50

class Core {
 public:
  static void spinms(uint32_t);
  static void start(uint32_t core, void (*func)(void));
  static void disableIRQ();
  static void enableIRQ();
  static void panic(const char* s);
  static Task* current[4];
  static void printList(ArrayList<Task *>* l);

  static Spinlock* runningQLock[4];
  static Spinlock* sleepingQLock;

  // static Vector<Task*> *runningQ[4];

  // static Task *runningQ[4][THREAD_N];
  static ArrayList<Task*>* runningQ[4];
  static ArrayList<Task*>* sleepingQ;
  static void preemptDisable();
  static void preemptEnable();
  static bool isPreamptable();
};

#endif