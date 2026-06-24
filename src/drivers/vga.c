#include <stdint.h>
#include <string.h>

#include <vga.h>


FILE vga_descriptor;
uint16_t *_vga_fb = (uint16_t*)VGA_FB_ADDR;
uint8_t _vga_attr = VGA_DEFCOLOR;

uint16_t _cursorX = 0;
uint16_t _cursorY = 0;

#define VGA_CPOS    ((_cursorY * VGA_WIDTH) + _cursorX)
#define VGA_MAXPOS  (((VGA_HEIGHT-1) * VGA_WIDTH) + VGA_WIDTH-1)

static void _vga_scroll(void) {
  memcpy(
    _vga_fb,
    _vga_fb + VGA_WIDTH,
    VGA_WIDTH * (VGA_HEIGHT - 1) * sizeof(uint16_t));
  --_cursorY;
}

void vga_clear(void) {
  const uint16_t c = ((uint16_t)_vga_attr << 8) | ' ';
  for (uint16_t i=0; i<=VGA_MAXPOS; ++i) {
    _vga_fb[i] = c;
  }
  _cursorY = 0;
  _cursorX = 0;
}

static int _vga_write(int fd, const void *buf, size_t count) {
  const char *src = (const char*)buf;
  char ch;

  while (count > 0) {
    ch = (char)*src;

    switch (ch) {
      case 0: break;

      case '\n': {
        _cursorX = 0;
        ++_cursorY;
        if (_cursorY >= VGA_HEIGHT)
          _vga_scroll();
        ++src; --count;
        break;
      }

      case '\r': {
        _cursorX = 0;
        ++src; --count;
        break;
      }

      case '\b': {
        _vga_fb[VGA_CPOS-1] = ((uint16_t)_vga_attr << 8) | (uint16_t)' ';
        _cursorX--;
        if (_cursorX >= VGA_WIDTH) {
          _cursorX = VGA_WIDTH - 1;
        }
        ++src; --count;
        break;
      }

      case '\t': {
        _cursorX = (_cursorX + 4) & ~3;
        if (_cursorX >= VGA_WIDTH) {_cursorX = 0; _cursorY++;}
        if (_cursorY >= VGA_HEIGHT) _vga_scroll();
        ++src; --count;
        break;
      }

      default: {
        _vga_fb[VGA_CPOS] = ((uint16_t)_vga_attr << 8) | (uint16_t)ch;

        ++src;
        --count;

        _cursorX++;
        if (_cursorX >= VGA_WIDTH) {
          _cursorX = 0;
          ++_cursorY;
          if (_cursorY >= VGA_HEIGHT)
            _vga_scroll();
        }
      }
    }
  }
  return count;
}

static int _vga_read(int fd, void *buf, size_t count) {
  return -1;
}


FILE *vga_init(int fd) {
  vga_descriptor.fd = fd;
  vga_descriptor.write  = _vga_write;
  vga_descriptor.read   = _vga_read;
  vga_descriptor.buffer = NULL;

  // Set file descriptor
  _file_descriptors[fd] = &vga_descriptor;
  return &vga_descriptor;
}

