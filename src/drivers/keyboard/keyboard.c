#include "keyboard.h"
#include "drivers/io/io.h"
#include "stdio.h"

uint8_t isShiftPressed = 0;

void keyboard_handler(void) {
  uint8_t scancode = inb(0x60);

  if (scancode == 0x2A) {
    isShiftPressed = 1;
    return;
  } else if (scancode == 0xAA) {
    isShiftPressed = 0;
    return;
  }

  if (scancode & 0x80) {
    // Released
    return;
  }

  // Pressed
  unsigned char ch;
  if (!isShiftPressed)
    ch = kbd_map_normal[scancode];
  else
    ch = kbd_map_shift[scancode];

  if (ch != 0) {
    putc(ch);
  }
}

