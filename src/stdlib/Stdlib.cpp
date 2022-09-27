#include "../include/Stdlib.h"

unsigned long Std::getCurrentEL() {
  register unsigned long x0 __asm__("x0");
  __asm__("mrs x0, CurrentEL;" : : : "%x0");
  return x0 >> 2;
}

/*  Format a string and print it on the screen, just like the libc
   function printf. */

#define INT_DIGITS 19
#define UINT_DIGITS 20

char *Std::itoa(int base, long d) {
  static char buf[INT_DIGITS + 2];
  char *p = buf + INT_DIGITS + 1; /* points to terminating '\0' */
  if (d >= 0) {
    do {
      *--p = '0' + (d % 10);
      d /= 10;
    } while (d != 0);
    return p;
  } else { /* i < 0 */
    do {
      *--p = '0' - (d % 10);
      d /= 10;
    } while (d != 0);
    *--p = '-';
  }
  return p;
}

uint32_t Std::djb33_hash(uint64_t in) {
  uint32_t h = 5381;
  uint32_t len = 0;
  while (len++ < 8) {
    /* h = 33 * h ^ s[i]; */
    h += (h << 5);
    h ^=  (in & (0xFF << len));
  }
  return h;
}

void Std::itoa1(char *buf, int base, long d) {
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
