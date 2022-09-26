#ifndef STDLIB_H
#define STDLIB_H

#include <cstddef>

extern "C" void _hang_forever();

class Std {
 public:
  static unsigned long getCurrentEL();
  static char *itoa(int base, long d);
  static void itoa1(char *buf, int base, long d);

  static unsigned long strlen(const char *s) {
    unsigned long len = 0;
    while (*s != '\0') {
      len++;
      s++;
    }
    return len;
  }

  static char *strcat(char *dest, const char *src) {
    size_t i, j;
    for (i = 0; dest[i] != '\0'; i++)
      ;
    for (j = 0; src[j] != '\0'; j++) dest[i + j] = src[j];
    dest[i + j] = '\0';
    return dest;
  }

  static char *strcpy(char *strDest, const char *strSrc) {
    char *temp = strDest;
    while ((*strDest++ = *strSrc++) != '\0')
      ;
    return temp;
  }
};

#endif