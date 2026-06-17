#ifndef _STDIO_H
#define _STDIO_H

#include <stdint.h>
#include <stddef.h>

#define MAX_DESCRIPTORS 16
#define STDIO   0
#define STDOUT  1
#define STDERR  2


// Read/write functions for FILE
typedef int (*fd_write)(int fd, const void *buf, size_t count);
typedef int (*fd_read)(int fd, void *buf, size_t count);

typedef struct {
  int fd;
  int flags;

  char *buffer;
  size_t buffer_size;
  size_t buffer_pos;

  // Functions
  fd_write  write;
  fd_read   read;
} FILE;


extern uint32_t errno;

extern FILE *_file_descriptors[MAX_DESCRIPTORS];
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

void k_stdio_init(void);

int putchar(int c);
int puts(const char *s);
int printf(const char *format, ...);  // TODO: Add va_list

int getchar(void);
int getline(char **lineptr, size_t *n, FILE *stream);

FILE *fopen(const char *pathname, const char *mode);
void flose(FILE *stream);

#endif // _STDIO_H

