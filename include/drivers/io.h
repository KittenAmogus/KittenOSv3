#ifndef _IO_H
#define _IO_H

#include <stdint.h>

// External assembly functions
void outsw(uint16_t port, void *buffer, uint32_t count);
void outb(uint16_t port, uint8_t data);
void outw(uint16_t port, uint16_t data);

void insw(uint16_t port, const void *buffer, uint32_t count);
uint8_t inb(uint16_t port);

#endif // _IO_H
