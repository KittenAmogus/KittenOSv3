#include <drivers/io.h>
#include <drivers/vga.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static volatile uint16_t *_vga_buffer = (volatile uint16_t *)VGA_ADDR;
static FILE _vga_fd;
static int _vga_cursor_x = 0;
int _vga_cursor_y = 0;
unsigned int _vga_anchor_pos = 0;

static uint16_t _vga_attr = 0x17;

#define VGA_SETCHAR(pos, c) _vga_buffer[pos] = (_vga_attr << 8) | c;
#define VGA_GETCHAR(pos) (_vga_buffer[pos] & 0xFF)
#define VGA_GETPOS (_vga_cursor_x + (_vga_cursor_y * VGA_WIDTH))

static void _vga_update_cursor(void) {
  uint16_t pos = VGA_GETPOS;

  // Set High
  outb(0x3D4, 14);
  outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));

  // Set Low
  outb(0x3D4, 15);
  outb(0x3D5, (uint8_t)(pos & 0xFF));
}

static void _vga_scroll(void) {
  --_vga_cursor_y;

  memcpy((void *)_vga_buffer, (void *)(_vga_buffer + VGA_WIDTH),
         VGA_WIDTH * (VGA_HEIGHT - 1) * sizeof(uint16_t));

  uint16_t vga_blank = (_vga_attr << 8) | ' ';
  for (int i = 0; i < VGA_WIDTH; ++i) {
    _vga_buffer[VGA_WIDTH * (VGA_HEIGHT - 1) + i] = vga_blank;
  }
}

static void _vga_writechar(char c) {
  switch (c) {
  case 0: {
    break;
  }

  case '\n': {
    _vga_cursor_x = 0;
    ++_vga_cursor_y;
    if (_vga_cursor_y >= VGA_HEIGHT)
      _vga_scroll();
    break;
  }

  case '\r': {
    _vga_cursor_x = 0;
    break;
  }

  case '\t': {
    int delta = ((_vga_cursor_x + 4) & ~3) - _vga_cursor_x;
    if (delta == 0)
      delta = 4;

    int start_pos = VGA_GETPOS;
    for (int i = 0; i < delta; ++i) {
      if ((_vga_cursor_x + i) < VGA_WIDTH) {
        VGA_SETCHAR(start_pos + i, ' ');
      }
    }

    _vga_cursor_x += delta;
    if (_vga_cursor_x >= VGA_WIDTH) {
      _vga_cursor_x = 0;
      ++_vga_cursor_y;
      if (_vga_cursor_y >= VGA_HEIGHT)
        _vga_scroll();
    }
    break;
  }

  case '\b': {
    if (VGA_GETPOS <= _vga_anchor_pos) {
      break;
    }

    if ((_vga_cursor_x & 3) == 0 && _vga_cursor_x >= 4) {
      int is_tab = 1;
      for (int i = 1; i <= 4; ++i) {
        if (VGA_GETCHAR(VGA_GETPOS - i) != ' ') {
          is_tab = 0;
          break;
        }
      }
      if (is_tab) {
        for (int i = 0; i < 4; ++i) {
          --_vga_cursor_x;
          VGA_SETCHAR(VGA_GETPOS, ' ');
        }
        break;
      }
    }

    --_vga_cursor_x;
    if (_vga_cursor_x < 0) {
      --_vga_cursor_y;
      _vga_cursor_x = VGA_WIDTH - 1;
    }
    VGA_SETCHAR(VGA_GETPOS, ' ');
    break;
  }

  default: {
    VGA_SETCHAR(VGA_GETPOS, c);
    ++_vga_cursor_x;
    if (_vga_cursor_x >= VGA_WIDTH) {
      ++_vga_cursor_y;
      _vga_cursor_x = 0;
      if (_vga_cursor_y >= VGA_HEIGHT)
        _vga_scroll();
    }
    break;
  }
  }
  _vga_update_cursor();
}

static int _vga_read(int fd, char *buffer, size_t count) { return -1; }

int _vga_write(int fd, const char *buffer, size_t count) {
  if (buffer == NULL || fd != _vga_fd.fd)
    return -1;
  if (count == 0)
    return 0;

  size_t n = count;
  while (n > 0) {
    _vga_writechar(*buffer);
    ++buffer;
    --n;
  }

  return count;
}

FILE *k_vga_create_stream(void) {
  _vga_fd.write = _vga_write;
  _vga_fd.read = _vga_read;
  return &_vga_fd;
}

void vga_clear(void) {
  for (int pos = 0; pos < VGA_WIDTH * VGA_HEIGHT; ++pos) {
    VGA_SETCHAR(pos, ' ');
  }
  _vga_cursor_x = 0;
  _vga_cursor_y = 0;
  _vga_anchor_pos = 0;
}

void vga_set_anchor(void) { _vga_anchor_pos = VGA_GETPOS; }
