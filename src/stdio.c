#include <stdio.h>
#include <string.h>
#include <stdlib.h>

uint32_t errno = 0;

FILE *_file_descriptors[MAX_DESCRIPTORS];
FILE *stdin   = NULL;
FILE *stdout  = NULL;
FILE *stderr  = NULL;

void k_stdio_init(void) {
  stdin  = _file_descriptors[0];
  stdout = _file_descriptors[1];
  stderr = _file_descriptors[2];
}

int putchar(int c) {
  if (stdout == NULL) return -1;
  char ch = (char)c;
  if (stdout->write(STDOUT, &ch, 1) != 1) return -1;
  return c;
}

int puts(const char *s) {
  if (s == NULL || stdout == NULL) return -1;

  size_t len = strlen(s);
  if (stdout->write(STDOUT, s, len) < 0) return -1;
  if (putchar('\n') != 0) return -1;
  return 0;
}

int printf(const char *format, ...) {
  return -1;
}

int getchar(void) {
  if (stdin == NULL) return -1;

  if (stdin->buffer != NULL) {
    if (stdin->buffer_pos < stdin->buffer_size) {
      return (int)stdin->buffer[stdin->buffer_pos++];
    }
    stdin->buffer_pos = 0;
    stdin->buffer_size = 0;
  }

  char ch;
  if (stdin->read(0, &ch, 1) != 1) return -1;
  return (int)ch;
}

int getline(char **lineptr, size_t *n, FILE *stream) {
  if (lineptr == NULL || n == NULL || stream == NULL) return -1;

  if (*lineptr == NULL) {
    *lineptr = malloc(128);
    if (*lineptr == NULL) return -1;
    *n = 128;
    memset(*lineptr, 0, 128);
  }

  size_t read = 0;
  int ch;

  while (true) {
    if (read >= *n - 2) {
      char *newline = realloc(*lineptr, *n << 1);
      if (newline == NULL) break;
      *lineptr = newline;
      *n = *n << 1;
    }

    ch = getchar();
 
    if (ch == -1) {
      if (read == 0) return -1;
      break;
    }

    if (ch == '\n') {
      (*lineptr)[read++] = '\n';
      break;
    }

    (*lineptr)[read++] = (char)ch;
  }

  (*lineptr)[read] = '\0';

  return read;
}

FILE *fopen(const char *pathname, const char *mode) {
  return NULL;
}

void fclose(FILE *stream) {
  return NULL;
}

