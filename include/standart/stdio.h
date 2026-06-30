#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>

#define MAX_FILE_DESCRIPTORS 256

// Function types
typedef int (*fd_read_t)(int fd, char *buffer, size_t count);
typedef int (*fd_write_t)(int fd, const char *buffer, size_t count);

typedef struct {
  int fd;
  int flags;
  char *buffer;

  size_t buffer_size;
  size_t buffer_pos;
  size_t read_pos;

  // Functions
  fd_read_t read;
  fd_write_t write;
} FILE;

// Default descriptors
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

void k_stdio_init(void);

int putchar(int c);
int puts(const char *s);

int getchar(void);
int getline(char **lineptr, size_t *n, FILE *stream);

FILE *fopen(const char *pathname, const char *mode);
void fclose(FILE *stream);

int fopendev(FILE *stream);

int printf(const char *format, ...);

#endif // _STDIO_H
