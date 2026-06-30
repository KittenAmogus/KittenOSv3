#include <drivers/io.h>
#include <stdint.h>
#include <stdio.h>

int is_shift_pressed = 0;

static const char kbd_us_map_normal[128] = {
    0,   27,  '1',  '2',  '3',  '4', '5', '6',  '7', '8', '9', '0',
    '-', '=', '\b', '\t', 'q',  'w', 'e', 'r',  't', 'y', 'u', 'i',
    'o', 'p', '[',  ']',  '\n', 0,   'a', 's',  'd', 'f', 'g', 'h',
    'j', 'k', 'l',  ';',  '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm',  ',',  '.',  '/', 0,   0,    0,   ' '};

static const char kbd_us_map_shift[128] = {
    0,   27,  '!',  '@',  '#',  '$', '%', '^', '&', '*', '(', ')',
    '_', '+', '\b', '\t', 'Q',  'W', 'E', 'R', 'T', 'Y', 'U', 'I',
    'O', 'P', '{',  '}',  '\n', 0,   'A', 'S', 'D', 'F', 'G', 'H',
    'J', 'K', 'L',  ':',  '"',  '~', 0,   '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M',  '<',  '>',  '?', 0,   0,   0,   ' '};

void irq_handler_kbd(void) {
  uint8_t scan = inb(0x60);

  // Shift
  if (scan == 0x2A || scan == 0x36) {
    is_shift_pressed = 1;
    return;
  } else if (scan == 0xAA || scan == 0xB6) {
    is_shift_pressed = 0;
    return;
  }

  // Release
  if (scan & 0x80)
    return;

  char ascii = (is_shift_pressed ? kbd_us_map_shift : kbd_us_map_normal)[scan];

  if (ascii != 0 && stdin != NULL && stdin->buffer != NULL) {

    /*
    if (ascii == '\b') {
      if (stdin->buffer_pos > 0) {
        --stdin->buffer_pos;
        putchar('\b');
      }
    }

    else {
      if (stdin->buffer_pos < stdin->buffer_size) {
        stdin->buffer[stdin->buffer_pos++] = ascii;
        putchar(ascii);
      }
    }
    */

    if (stdin->buffer_pos < stdin->buffer_size) {
      if (stdin->buffer_pos > 0 || ascii != '\b') {
        stdin->buffer[stdin->buffer_pos++] = ascii;
        putchar(ascii);
      }
    }
  }
}
