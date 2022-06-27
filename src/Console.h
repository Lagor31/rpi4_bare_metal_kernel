#ifndef CONSOLE_H
#define CONSOLE_H

class Console {
 public:
  // virtual void write(const char *s, ...) = 0;
  virtual void write(char) = 0;
  virtual void flush() = 0;
  static void print(const char *s, ...);
  static void setKernelConsole(Console *in);

 private:
  static Console *kernel_console;
};

#endif