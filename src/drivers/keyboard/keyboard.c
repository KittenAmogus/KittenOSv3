#include "keyboard.h"
#include "drivers/io/io.h"
#include "stdio.h"

uint8_t isShiftPressed = 0;

char kbd_buffer[KBD_BUFFER_SIZE];
unsigned int kbd_head = 0;
unsigned int kbd_tail = 0;


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
    // Add char to buffer
    unsigned int head_nxt = (kbd_head + 1) % KBD_BUFFER_SIZE;
    if (head_nxt != kbd_tail) {
      kbd_buffer[kbd_head] = ch;
      kbd_head = head_nxt;
    }
  }
}

