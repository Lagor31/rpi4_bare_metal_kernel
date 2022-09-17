#ifndef CORE_H
#define CORE_H

#include <stddef.h>
#include <stdint.h>

class Core {
 public:
  static void spinms(uint64_t);
  static void start(uint32_t core, void (*func)(void));
  static void disableIRQ();
  static void enableIRQ();

};

#endif