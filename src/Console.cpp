#include "Console.h"

#include <cstdarg>

Console *Console::kernel_console;

/*  Format a string and print it on the screen, just like the libc
   function printf. */
static void itoa(char *buf, int base, int d) {
  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;

  /*  If %d is specified and D is minus, put ‘-’ in the head. */
  if (base == 'd' && d < 0) {
    *p++ = '-';
    buf++;
    ud = -d;
  } else if (base == 'x')
    divisor = 16;

  /*  Divide UD by DIVISOR until UD == 0. */
  do {
    int remainder = ud % divisor;

    *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
  } while (ud /= divisor);

  /*  Terminate BUF. */
  *p = 0;

  /*  Reverse BUF. */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2) {
    char tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
    p1++;
    p2--;
  }
}

void Console::print(const char *format, ...) {
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
          itoa(buf, c, n);
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
