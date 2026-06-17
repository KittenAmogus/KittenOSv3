#include <stdint.h>
#include <stdio.h>
#include <io.h>

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

  if (ascii != 0) {
    // Fill stdin
    if (stdin != NULL && stdin->buffer != NULL) {
      if (ascii == '\b') {
        if (stdin->buffer_pos != 0)
          stdin->buffer_pos--;
      }
      if (stdin->buffer_pos < stdin->buffer_size) {
        stdout->write(1, &ascii, 1);
        if (ascii != '\b')
          stdin->buffer[stdin->buffer_pos++] = ascii;
      }
    }
  }
}

void kb_init(void) {
  inb(0x60);
}

