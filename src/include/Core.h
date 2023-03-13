#ifndef CORE_H
#define CORE_H

#include <stddef.h>
#include <stdint.h>

#include "List.h"
#include "Task.h"

#define THREAD_N 200
#define NUM_CORES 4
#define current (Core::currentTask[get_core()])
#define PRIORITIES 21

class Core {
 public:
  static void spinms(uint32_t);
  static void start(uint32_t core, void (*func)(void));
  static void disableIRQ();
  static void enableIRQ();
  static void panic(const char* s);
  static Task* currentTask[NUM_CORES];
  static void printList(ArrayList<Task*>* l);

  static Spinlock* runningQLock[NUM_CORES];
  static Spinlock* sleepingQLock;

  static ArrayList<Task*>* runningQ[NUM_CORES][PRIORITIES];
  static ArrayList<Task*>* sleepingQ;
  static void preemptDisable();
  static void preemptEnable();
  static bool isPreamptable();
};

#endif