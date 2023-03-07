#ifndef FB_H
#define FB_H

#include <stddef.h>
#include <stdint.h>

#include "Lists/SinglyLinkedList.hpp"
#include "Spinlock.h"
typedef struct circle_t {
  uint32_t radius;
  uint32_t x;
  uint32_t y;
  uint32_t attr;
  uint8_t fill;
} Circle;
using SD::Lists::SinglyLinkedList;
uint8_t getCoreColor(uint32_t coreNumber);
void paintCircle(Circle *c);
void FBInit();
void drawPixel(int x, int y, unsigned char attr);
bool drawChar(unsigned char ch, int x, int y, unsigned char attr);
void drawString(int x, int y, const char *s, unsigned char attr);
void drawRect(int x1, int y1, int x2, int y2, unsigned char attr, int fill);
void drawCircle(int x0, int y0, int radius, unsigned char attr, int fill);
void drawLine(int x1, int y1, int x2, int y2, unsigned char attr);

extern unsigned char *kernelFb;
extern SinglyLinkedList<Circle *> *circles;
extern Spinlock *fb_lock;

extern unsigned int width, height, pitch, isrgb;
extern unsigned char *fb;
#endif