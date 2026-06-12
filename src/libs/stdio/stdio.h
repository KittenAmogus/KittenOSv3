#ifndef STDIO_H
#define STDIO_H

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

// Std-like funcs
// Output
uint8_t  putc(char c);
uint32_t puts(const char *c);
uint32_t printf(const char *format, ...);

// Input
char getchar(void);
char *getline(uint32_t limit);

// Non-std funcs
void clear_screen(void);

uint32_t putstr(const char *c);
uint32_t putint(int32_t x);
uint32_t puthex(int32_t x);
void cursor_pos(uint16_t x, uint16_t y);

#endif // STDIO_H

