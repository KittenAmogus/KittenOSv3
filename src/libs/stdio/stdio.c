#include "stdio.h"

// VGA Framebuffer addr
#define VGA_BUFFER  0x000B8000
#define VGA_DEFAULT_ATTRS 0x82
#define VGA_WIDTH   80
#define VGA_HEIGHT  24

#define FB_FROM_CURSOR(x, y)  (y * VGA_WIDTH + x)

static volatile unsigned short * const _fb =\
  (volatile unsigned short *)VGA_BUFFER;
static unsigned char _vgaAttrs = 0x00;

static unsigned short _cursorX;
static unsigned short _cursorY;

int putc(int c) {
  unsigned char ch = (unsigned char)c;

  switch (ch) {
    case '\n':
      _cursorX = 0;
      ++_cursorY; break;

    case '\r':
      _cursorX = 0;
      break;

    default:
      unsigned short ac = (_vgaAttrs << 8) | ch;
      _fb[FB_FROM_CURSOR(_cursorX, _cursorY)] = ac;

      _cursorX++;
      if (_cursorX >= VGA_WIDTH) {
        _cursorX = 0;
        _cursorY++;
      }
  }

  return c;
}

int puts(const char *s) {
  while (*s != 0) {
    putc(*s);
    ++s;
  }
  putc('\n');

  return 0;
}

void vgaAttrs(const char fg, const char bg) {
  _vgaAttrs = (bg << 8) | fg;
}

