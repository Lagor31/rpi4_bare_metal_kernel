#ifndef CONSOLE_H
#define CONSOLE_H

#include "Spinlock.h"

class Console {
 public:
  virtual void write(char) = 0;
  virtual void flush() = 0;
  virtual unsigned char readChar() = 0;
  static void print(const char *s, ...);
  static void print_no_lock(const char *s, ...);
  static void setKernelConsole(Console *in);
  static Console *getKernelConsole();
};

#endif