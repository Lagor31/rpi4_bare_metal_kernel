#include "Console.h"

#include <cstdarg>

#include "stdlib/Stdlib.h"


using namespace ltl::console;

Console *Console::kernel_console = nullptr;

void Console::print(const char *format, ...) {
  if (kernel_console == nullptr) return;

  char **arg = (char **)&format;
  int c;
  char buf[40];
  int n;

  va_list args;
  va_start(args, format);

  arg++;

  while ((c = *format++) != 0) {
    if (c != '%')
      kernel_console->write(c);
    else {
      char *p, *p2;
      int pad0 = 0, pad = 0;

      c = *format++;
      if (c == '0') {
        pad0 = 1;
        c = *format++;
      }

      if (c >= '0' && c <= '9') {
        pad = c - '0';
        c = *format++;
      }

      switch (c) {
        case 'd':
        case 'u':
        case 'x':
          n = va_arg(args, int);
          Std::itoa(buf, c, n);
          p = buf;
          goto string;
          break;

        case 's':
          p = va_arg(args, char *);
          if (!p) p = (char *)"(null)";

        string:
          for (p2 = p; *p2; p2++)
            ;
          for (; p2 < p + pad; p2++) kernel_console->write(pad0 ? '0' : ' ');
          while (*p) kernel_console->write(*p++);
          break;

        default:
          int c = va_arg(args, int);
          kernel_console->write(c);
          break;
      }
    }
  }
  va_end(args);
};

void Console::setKernelConsole(Console *in) { kernel_console = in; };
