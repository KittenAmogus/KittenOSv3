#ifndef STDIO_H
#define STDIO_H

// Std-like funcs
// Output
int putc(int c);
int puts(const char *c);
int printf(const char *format, ...);

// Input
char getchar(void);
char *getline(unsigned int limit);

// Non-std funcs
int putstr(const char *c);
int putint(const unsigned int x);
int puthex(const unsigned int x);
void vgaAttrs(const char fg, const char bg);
void cursorPos(unsigned short x, unsigned short y);

#endif // STDIO_H

