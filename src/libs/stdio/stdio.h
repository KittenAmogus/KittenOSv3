#ifndef STDIO_H
#define STDIO_H

// VGA print functions
int putc(int c);
int puts(const char *c);
void vgaAttrs(const char fg, const char bg);

void cursorPos(unsigned short x, unsigned short y);

#endif // STDIO_H

