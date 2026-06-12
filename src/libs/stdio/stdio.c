#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"

#include "drivers/io/io.h"
#include "drivers/keyboard/keyboard.h"

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

  unsigned short ac;
  switch (ch) {
    case '\n': _cursorX = 0;
      ++_cursorY;
      fb_move_cursor(FB_FROM_CURSOR(_cursorX, _cursorY));
      break;

    case '\r':
      _cursorX = 0;
      break;

    case '\b':
      // Move cursor back
      _cursorX--;
      if (_cursorX >= VGA_WIDTH) {
        _cursorX = VGA_WIDTH - 1;
        _cursorY--;
      }

      // Erase last char
      ac = (_vgaAttrs << 8) | 0;
      _fb[FB_FROM_CURSOR(_cursorX, _cursorY)] = ac;

      fb_move_cursor(FB_FROM_CURSOR(_cursorX, _cursorY));
      break;

    case '\t':
      // Tabs...
      break;

    default:
      ac = (_vgaAttrs << 8) | ch;
      _fb[FB_FROM_CURSOR(_cursorX, _cursorY)] = ac;

      _cursorX++;
      if (_cursorX >= VGA_WIDTH) {
        _cursorX = 0;
        _cursorY++;
      }
      fb_move_cursor(FB_FROM_CURSOR(_cursorX, _cursorY));
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

int putint(unsigned int x) {
  int wr = 0;
  for (unsigned char i=0; i<8; i++) {
    unsigned char c = (x >> (28 - (i << 2)) & 0x0F);
    c += '0';
    putc(c);
    ++wr;
  }

  return wr;
}

int puthex(unsigned int x) {
  for (unsigned char i=0; i<8; i++) {
    unsigned char c = (x >> (28 - (i << 2)) & 0x0F);

    if (c < 10) {
      c += '0';
    }
    else {
      c += 'A' - 10;
    }

    putc(c);
  }

  return 8;
}

int putstr(const char *s) {
  int wr;
  while (*s != 0) {
    putc(*s);
    ++s;
    ++wr;
  }

  return wr;
}

int printf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  int written = 0;

  while (*format != 0) {
    if (*format == '%') {
      ++format;

      switch (*format) {
        case 'c': {
          char c = (char)va_arg(args, int);
          putc(c);
          ++written;
          break;
        }

        case 's': {
          char *s = va_arg(args, char*);
          written += putstr(s);
          break;
        }

        case 'x': {
          unsigned int x = (unsigned int)va_arg(args, unsigned int);
          written += puthex(x);
          break;
        }

        case 'd': {
          unsigned int d = (unsigned int)va_arg(args, unsigned int);
          written += putint(d);
          break;
        }

        case '%': {
          putc('%');
          ++written;
          break;
        }

        default: {
          putc('%');
          putc(*format);
          written += 2;
          break;
        }
      }
    } else {
      putc(*format);
      ++written;
    }
    ++format;
  }

  va_end(args);
  return written;
}

void vgaAttrs(const char fg, const char bg) {
  _vgaAttrs = (bg << 4) | fg;
}

void cursorPos(unsigned short x, unsigned short y) {
  _cursorX = x % VGA_WIDTH;
  _cursorY = y % VGA_HEIGHT;
}


/* === INPUT === */

char getchar(void) {
  // Wait for buffer to have at least one char
  while (kbd_head == kbd_tail) {
    asm volatile ("nop");
  }

  // First char in ring buffer
  char ch = kbd_buffer[kbd_tail];
  kbd_tail = (kbd_tail + 1) % KBD_BUFFER_SIZE;
  return ch;
}

char *getline(size_t limit) {
  char *line = malloc(limit);
  if (line == NULL) return NULL;

  size_t count = 0;
  char ch;

  while (count < limit - 1) {
    ch = getchar();
    if (ch == '\n' || ch == '\r') {
      putc('\n');
      break;
    }
    else if (ch == '\b') {

      if (count == 0) continue;

      // Move cursor back
      _cursorX--;
      if (_cursorX >= VGA_WIDTH) {
        _cursorX = VGA_WIDTH - 1;
        _cursorY--;
      }

      // Erase last char
      unsigned short ac = (_vgaAttrs << 8) | 0;
      _fb[FB_FROM_CURSOR(_cursorX, _cursorY)] = ac;

      fb_move_cursor(FB_FROM_CURSOR(_cursorX, _cursorY));

      count -= (count > 0);
      continue;
    }

    line[count++] = ch;
    putc(ch);
  }

  line[count] = 0;
  return line;
}

