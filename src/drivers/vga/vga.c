#include "vga.h"
#include "drivers/io/io.h"

uint8_t _vga_attrs = 0x00;
volatile uint16_t * const _fb =\
  (volatile uint16_t *)VGA_BUFFER;

extern uint16_t _cursor_y;
extern uint16_t _cursor_x;

void vga_attrs(const uint8_t fg, const uint8_t bg) {
  _vga_attrs = (bg << 4) | fg;
}

void fb_move_cursor(uint16_t pos) {
  outb(FB_COMMAND_PORT, FB_HIGH_BYTE_COMMAND);
  outb(FB_DATA_PORT, (pos >> 8) & 0xFF);
  outb(FB_COMMAND_PORT, FB_LOW_BYTE_COMMAND);
  outb(FB_DATA_PORT, pos & 0xFF);
}

void fb_scroll(void) {
  for (uint32_t i=0; i<(VGA_HEIGHT - 1) * VGA_WIDTH; ++i) {
    _fb[i] = _fb[i + VGA_WIDTH];  // next line
  }

  uint16_t blank = (_vga_attrs << 8) | 0;
  for (uint32_t i=(VGA_HEIGHT - 1) * VGA_WIDTH;
    i < VGA_HEIGHT * VGA_WIDTH; ++i) {
    _fb[i] = blank;
  }

  _cursor_y = (VGA_HEIGHT - 1);
}

