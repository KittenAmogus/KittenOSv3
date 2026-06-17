#ifndef IO_H
#define IO_H

#include <stdint.h>

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void outsw(uint16_t port, const void *addr, uint32_t count);
void insw(uint16_t port, void *addr, uint32_t count);

#endif // IO_H

