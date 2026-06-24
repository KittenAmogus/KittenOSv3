#ifndef VGA_H
#define VGA_H

#include <stdio.h>

#define VGA_WIDTH   80
#define VGA_HEIGHT  25

#define VGA_DEFCOLOR  0x17  // Gray on blue
#define VGA_FB_ADDR   0x000B8000


extern FILE vga_descriptor;

void vga_clear(void);
FILE *vga_init(int fd);

#endif // VGA_H

