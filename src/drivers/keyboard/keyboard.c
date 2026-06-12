#include "keyboard.h"
#include "drivers/io/io.h"
#include "stdio.h"

uint8_t is_shift_pressed = 0;

uint8_t kbd_buffer[KBD_BUFFER_SIZE];
uint32_t kbd_head = 0;
uint32_t kbd_tail = 0;


void keyboard_handler(void) {
  uint8_t scancode = inb(0x60); // Key scancode

  // Shift
  if (scancode == 0x2A) {         // Press
    is_shift_pressed = 1;
    return;
  } else if (scancode == 0xAA) {  // Release
    is_shift_pressed = 0;
    return;
  }

  // Key release
  if (scancode & 0x80)
    return;

  // Get ascii representation to save
  uint8_t ascii = (!is_shift_pressed ? kbd_map_normal : kbd_map_shift)[scancode];

  // Non-printable
  if (ascii == 0) return;

  // Add ascii to buffer
  uint32_t kbd_head_n = (kbd_head + 1) % KBD_BUFFER_SIZE;
  if (kbd_head_n == kbd_tail) return; // No space
  kbd_buffer[kbd_head] = ascii;
  kbd_head = kbd_head_n;
}

