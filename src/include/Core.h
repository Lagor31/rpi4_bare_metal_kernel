#ifndef CORE_H
#define CORE_H

#include <stddef.h>
#include <stdint.h>

#include "Task.h"
#include "Vector.h"
#define THREAD_N 500
class Core {
 public:
  static void spinms(uint32_t);
  static void start(uint32_t core, void (*func)(void));
  static void disableIRQ();
  static void enableIRQ();
  static void switchTo(Task *next);
  static Task *current[4];
  // static Vector<Task*> *runningQ[4];
  static Task *runningQ[4][THREAD_N];

  static void preemptDisable();
  static void preemptEnable();
  static bool isPreamptable();
};



#endif