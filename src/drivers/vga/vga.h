#ifndef VGA_H
#define VGA_H

#include "stdint.h"

// VGA Framebuffer addr
#define VGA_BUFFER  0x000B8000
#define VGA_DEFAULT_ATTRS 0x82
#define VGA_WIDTH   80
#define VGA_HEIGHT  24

// Framebuffer cursor
#define FB_COMMAND_PORT 0x3D4
#define FB_DATA_PORT    0x3D5
#define FB_HIGH_BYTE_COMMAND  14
#define FB_LOW_BYTE_COMMAND   15

void fb_scroll(void);
void fb_move_cursor(uint16_t pos);
void vga_attrs(const uint8_t fg, const uint8_t bg);

#endif // VGA_H

