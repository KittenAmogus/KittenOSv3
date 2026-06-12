#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"

#include "drivers/io/io.h"
#include "drivers/vga/vga.h"
#include "drivers/keyboard/keyboard.h"

#define FB_FROM_CURSOR(x, y)  (y * VGA_WIDTH + x)

extern volatile uint16_t * const _fb;
extern uint8_t _vga_attrs;

uint16_t _cursor_x;
uint16_t _cursor_y;


uint8_t putc(char c) {
  if (_cursor_y >= VGA_HEIGHT)
    fb_scroll();

  uint16_t ac;
  switch (c) {
    case '\n': {
      _cursor_x = 0;
      ++_cursor_y;
      fb_move_cursor(FB_FROM_CURSOR(_cursor_x, _cursor_y));
      break;
    }

    case '\r': {
      _cursor_x = 0;
      break;
    }

    case '\b': {
      // Move cursor back
      _cursor_x--;
      if (_cursor_x >= VGA_WIDTH) {
        _cursor_x = VGA_WIDTH - 1;
        _cursor_y--;
      }

      // Erase last char
      ac = (_vga_attrs << 8) | 0;
      _fb[FB_FROM_CURSOR(_cursor_x, _cursor_y)] = ac;

      fb_move_cursor(FB_FROM_CURSOR(_cursor_x, _cursor_y));
      break;
    }

    case '\t': {
      // TODO: Tabs...
      break;
    }

    default: {
      ac = (_vga_attrs << 8) | c;
      _fb[FB_FROM_CURSOR(_cursor_x, _cursor_y)] = ac;

      _cursor_x++;
      if (_cursor_x >= VGA_WIDTH) {
        _cursor_x = 0;
        _cursor_y++;
      }
      fb_move_cursor(FB_FROM_CURSOR(_cursor_x, _cursor_y));
    }
  }

  return c;
}

uint32_t puts(const char *c) {
  while (*c != 0) {
    putc(*c);
    ++c;
  }
  putc('\n');

  return 0;
}

uint32_t putint(int32_t x) {
  char buff[12];
  uint32_t i = 0;
  uint32_t wr = 0;

  // Negative
  if (x < 0) {
    putc('-');
    ++wr;
    x = -x;
  }

  // Zero
  if (x == 0) {
    putc('0');
    return 1;
  }

  // Calc digits
  while (x > 0) {
    buff[i++] = (x % 10) + '0';
    x /= 10;
  }

  // Write digits
  while (i > 0) {
    putc(buff[--i]);
    ++wr;
  }

  return wr;
}

uint32_t puthex(int32_t x) {
  char buff[12];
  uint32_t i = 0;
  uint32_t wr = 0;

  // Negative
  if (x < 0) {
    putc('-');
    ++wr;
    x = -x;
  }

  // Zero
  if (x == 0) {
    putc('0');
    return 1;
  }

  // Calc hex digits
  while (x > 0) {
    uint8_t rem = x & 0x0F;
    buff[i++] = (rem < 10) ? (rem + '0') : (rem - 10 + 'A');
    x >>= 4;
  }

  // Write digits
  while (i > 0) {
    putc(buff[--i]);
    ++wr;
  }

  return wr;
}


uint32_t putstr(const char *s) {
  uint32_t wr;
  while (*s != 0) {
    putc(*s);
    ++s;
    ++wr;
  }

  return wr;
}

uint32_t printf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  int written = 0;

  while (*format != 0) {
    if (*format == '%') {
      ++format;

      switch (*format) {
        case 'c': {
          char c = (char)va_arg(args, uint32_t);
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
          unsigned int x = (unsigned int)va_arg(args, uint32_t);
          written += puthex(x);
          break;
        }

        case 'd': {
          unsigned int d = (unsigned int)va_arg(args, uint32_t);
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

void cursor_pos(uint16_t x, uint16_t y) {
  _cursor_x = x % VGA_WIDTH;
  _cursor_y = y % VGA_HEIGHT;
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
      _cursor_x--;
      if (_cursor_x >= VGA_WIDTH) {
        _cursor_x = VGA_WIDTH - 1;
        _cursor_y--;
      }

      // Erase last char
      unsigned short ac = (_vga_attrs << 8) | 0;
      _fb[FB_FROM_CURSOR(_cursor_x, _cursor_y)] = ac;

      fb_move_cursor(FB_FROM_CURSOR(_cursor_x, _cursor_y));

      count -= (count > 0);
      continue;
    }

    line[count++] = ch;
    putc(ch);
  }

  line[count] = 0;
  return line;
}

void clear_screen(void) {
  cursor_pos(0, 0);
  for (int a=0; a<(80 * 25); ++a) {
    putc(0);
  }
  cursor_pos(0, 0);
}

