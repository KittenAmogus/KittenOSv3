#ifndef _VGA_H
#define _VGA_H

#include <stdio.h>

#define VGA_ADDR 0x000B8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

void vga_clear(void);
void vga_set_anchor(void);
FILE *k_vga_create_stream(void);

#endif // _VGA_H
