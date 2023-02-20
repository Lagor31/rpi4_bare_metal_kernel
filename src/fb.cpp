#include "include/fb.h"

#include "include/Console.h"
#include "include/Core.h"
#include "include/Mem.h"
#include "include/Spinlock.h"
#include "include/io.h"
#include "include/mb.h"
#include "include/terminal.h"

using SD::Lists::SinglyLinkedList;

SinglyLinkedList<Circle *> *circles;

unsigned int width, height, pitch, isrgb;
unsigned char *fb;
unsigned char *kernelFb;

Spinlock *fb_lock;

void fb_init() {
  fb_lock = new Spinlock();
  mbox[0] = 35 * 4;  // Length of message in bytes
  mbox[1] = MBOX_REQUEST;

  mbox[2] = MBOX_TAG_SETPHYWH;  // Tag identifier
  mbox[3] = 8;                  // Value size in bytes
  mbox[4] = 0;
  mbox[5] = 1920;  // Value(width)
  mbox[6] = 1080;  // Value(height)

  mbox[7] = MBOX_TAG_SETVIRTWH;
  mbox[8] = 8;
  mbox[9] = 8;
  mbox[10] = 1920;
  mbox[11] = 1080 + 200;

  mbox[12] = MBOX_TAG_SETVIRTOFF;
  mbox[13] = 8;
  mbox[14] = 8;
  mbox[15] = 0;  // Value(x)
  mbox[16] = 0;  // Value(y)

  mbox[17] = MBOX_TAG_SETDEPTH;
  mbox[18] = 4;
  mbox[19] = 4;
  mbox[20] = 32;  // Bits per pixel

  mbox[21] = MBOX_TAG_SETPXLORDR;
  mbox[22] = 4;
  mbox[23] = 4;
  mbox[24] = 1;  // RGB

  mbox[25] = MBOX_TAG_GETFB;
  mbox[26] = 8;
  mbox[27] = 8;
  mbox[28] = 4096;  // FrameBufferInfo.pointer
  mbox[29] = 0;     // FrameBufferInfo.size

  mbox[30] = MBOX_TAG_GETPITCH;
  mbox[31] = 4;
  mbox[32] = 4;
  mbox[33] = 0;  // Bytes per line

  mbox[34] = MBOX_TAG_LAST;

  // Check call is successful and we have a pointer with depth 32
  if (mbox_call(MBOX_CH_PROP) && mbox[20] == 32 && mbox[28] != 0) {
    mbox[28] &= 0x3FFFFFFF;  // Convert GPU address to ARM address
    width = mbox[10];        // Actual physical width
    height = mbox[11];       // Actual physical height
    pitch = mbox[33];        // Number of bytes per line
    isrgb = mbox[24];        // Pixel order
    fb = 0xffff000000000000 + (unsigned char *)((long)mbox[28]);
    kernelFb = (uint8_t *)GlobalKernelAlloc::alloc(4 * width * height);

    circles = new SinglyLinkedList<Circle *>();

    // for (int c = 0; c < 4 * width * height; ++c) fb[c] = 0xeE;
    Console::print("Frame buffer allocated!\nW: %d H: %d Pitch: %d RGB %d\n",
                   width, height, pitch, isrgb);
  } else
    Core::panic("Error allocating framebuffer!\n");
}

void drawPixel(int x, int y, unsigned char attr) {
  int offs = (y * pitch) + (x * 4);
  // fb_lock->getLock();
  *((unsigned int *)(fb + offs)) = vgapal[attr & 0x0f];
  RPIQ_INVAL_DCACHE(fb + offs);
  RPIQ_MEM_BARRIER();
  // fb_lock->release();
}

void drawRect(int x1, int y1, int x2, int y2, unsigned char attr, int fill) {
  int y = y1;

  while (y <= y2) {
    int x = x1;
    while (x <= x2) {
      if ((x == x1 || x == x2) || (y == y1 || y == y2))
        drawPixel(x, y, attr);
      else if (fill)
        drawPixel(x, y, (attr & 0xf0) >> 4);
      x++;
    }
    y++;
  }
}

void drawLine(int x1, int y1, int x2, int y2, unsigned char attr) {
  int dx, dy, p, x, y;

  dx = x2 - x1;
  dy = y2 - y1;
  x = x1;
  y = y1;
  p = 2 * dy - dx;

  while (x < x2) {
    if (p >= 0) {
      drawPixel(x, y, attr);
      y++;
      p = p + 2 * dy - 2 * dx;
    } else {
      drawPixel(x, y, attr);
      p = p + 2 * dy;
    }
    x++;
  }
}

void paintCircle(Circle *c) {
  // Core::preemptDisable();

  fb_lock->getLock();
  Core::disableIRQ();
  circles->insert(c);
  Core::enableIRQ();

  fb_lock->release();
  // Core::preemptEnable();
}

void drawCircle(int x0, int y0, int radius, unsigned char attr, int fill) {
  int x = radius;
  int y = 0;
  int err = 0;
  // fb_lock->getLock();

  while (x >= y) {
    if (fill) {
      drawLine(x0 - y, y0 + x, x0 + y, y0 + x, (attr & 0xf0) >> 4);
      drawLine(x0 - x, y0 + y, x0 + x, y0 + y, (attr & 0xf0) >> 4);
      drawLine(x0 - x, y0 - y, x0 + x, y0 - y, (attr & 0xf0) >> 4);
      drawLine(x0 - y, y0 - x, x0 + y, y0 - x, (attr & 0xf0) >> 4);
    }
    drawPixel(x0 - y, y0 + x, attr);
    drawPixel(x0 + y, y0 + x, attr);
    drawPixel(x0 - x, y0 + y, attr);
    drawPixel(x0 + x, y0 + y, attr);
    drawPixel(x0 - x, y0 - y, attr);
    drawPixel(x0 + x, y0 - y, attr);
    drawPixel(x0 - y, y0 - x, attr);
    drawPixel(x0 + y, y0 - x, attr);

    if (err <= 0) {
      y += 1;
      err += 2 * y + 1;
    }

    if (err > 0) {
      x -= 1;
      err -= 2 * x + 1;
    }
  }
  // fb_lock->release();
}

bool drawChar(unsigned char ch, int x, int y, unsigned char attr) {
  static char *g;

  switch (ch) {
    case 'f':
      g = f_bits;
      break;
    case 'e':
      g = e_bits;
      break;
    case 'd':
      g = d_bits;
      break;
    case 'F':
      g = F_bits;
      break;
    default:
      return false;
  }

  uint32_t pX = x;
  uint32_t pY = y;
  char *glyph = g;

  for (int i = 0; i < FONT_HEIGHT; i++) {
    for (int j = 0; j < FONT_BPL; j++) {
      for (int l = 0; l < 8; l++) {
        unsigned char mask = 1 << l;
        unsigned char col =
            (glyph[i * 2 + j] & mask) ? attr & 0x0f : (attr & 0xf0) >> 4;
        drawPixel(pX + l, pY + i, col);
      }
      pX += 8;
    }
    pX = x;
  }
  return true;
}

void drawString(int x, int y, char *s, unsigned char attr) {
  while (*s) {
    if (*s == '\r') {
      x = 0;
    } else if (*s == '\n') {
      x = 0;
      y += FONT_HEIGHT;
    } else {
      drawChar(*s, x, y, attr);
      x += FONT_WIDTH;
    }
    s++;
  }
}