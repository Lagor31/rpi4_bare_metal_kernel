#ifndef CORE_H
#define CORE_H

#include <stddef.h>
#include <stdint.h>

class Core {
 public:
  static void spinms(uint64_t);
};

#endif