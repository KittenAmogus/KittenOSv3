#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>

#define KB_MAX_BUFFER 1024

static const char kbd_us_normal[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
  '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,   '*',   0,
   ' '
};

static const char kbd_us_shift[58] = {
  0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
  0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0,
  '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0,   '*',   0,
  ' '
};

static uint8_t is_shift_pressed = 0;

FILE _kb_descriptor;
static size_t _kb_read_pos = 0;

static int _kb_private_write(int fd, const void *buf, size_t count) {
  if (_kb_descriptor.buffer == NULL || buf == NULL) return -1;
  const char *src = (const char*)buf;

  while (count > 0) {
    char ch = *src;

    if (ch == '\b') {
      if (_kb_descriptor.buffer_pos > _kb_read_pos) {
        _kb_descriptor.buffer_pos--;
      }
      --count; ++src; continue;
    }

    if (_kb_descriptor.buffer_pos < _kb_descriptor.buffer_size) {
      _kb_descriptor.buffer[_kb_descriptor.buffer_pos++] = ch;
    }

    --count; ++src;
  }
  return 0;
}

static int _kb_read(int fd, void *buf, size_t count) {
  if (fd != _kb_descriptor.fd || buf == NULL) return -1;
  char *dest = (char*)buf;
  size_t read_bytes = 0;

  while (count > 0 && (_kb_read_pos < _kb_descriptor.buffer_pos)) {
    *dest = _kb_descriptor.buffer[_kb_read_pos++];
    --count;
    ++read_bytes;
    ++dest;
  }

  if (_kb_read_pos == _kb_descriptor.buffer_pos) {
    _kb_descriptor.buffer_pos = 0;
    _kb_read_pos = 0;
  }

  return read_bytes;
}


void keyboard_handler(void) {
  uint8_t scancode = inb(0x60);

  if (scancode == 0x2A || scancode == 0x36) { // L/R shifts
    is_shift_pressed  = 1;
    return;
  }

  if (scancode == 0xAA || scancode == 0xB6) { // L/R shifts (release)
    is_shift_pressed  = 0;
    return;
  }

  if (scancode & 0x80) {  // Release
    return;
  }

  // Not fits
  if (scancode >= 58) {
    return;
  }

  char ascii = (is_shift_pressed ? kbd_us_shift : kbd_us_normal)[scancode];

  // Fill stdin
  if (ascii != 0) {
    _kb_private_write(_kb_descriptor.fd, &ascii, 1);
  }
}


static int _kb_write(int fd, const void *buf, size_t count) {
  return -1;
}

FILE *kb_init(int fd) {
  _kb_descriptor.fd = fd;
  _kb_descriptor.write  = _kb_write;
  _kb_descriptor.read   = _kb_read;
  _kb_descriptor.buffer = malloc(KB_MAX_BUFFER);

  if (_kb_descriptor.buffer != NULL)
    _kb_descriptor.buffer_size  = KB_MAX_BUFFER;
  else
    _kb_descriptor.buffer_size  = 0;

  _kb_descriptor.buffer_pos = 0;

  // Set file descriptor
  _file_descriptors[fd] = &_kb_descriptor;
  return &_kb_descriptor;
}

