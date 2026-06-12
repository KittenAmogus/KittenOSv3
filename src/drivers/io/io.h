#ifndef IO_H
#define IO_H

void outb(unsigned short port, unsigned char data);
unsigned char inb(unsigned short port);

void fb_move_cursor(unsigned short pos);

#endif // IO_H

