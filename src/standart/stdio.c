#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *_file_descriptors[MAX_FILE_DESCRIPTORS] = {NULL};
FILE *stdin = NULL;
FILE *stdout = NULL;
FILE *stderr = NULL;

extern FILE *k_vga_create_stream(void);

void k_stdio_init(void) {
  _file_descriptors[1] = k_vga_create_stream();
  _file_descriptors[1]->fd = 1;

  stdin = _file_descriptors[0];
  stdout = _file_descriptors[1];
  stderr = _file_descriptors[2];
}

int putchar(int c) {
  if (stdout == NULL)
    return -1;

  if (stdout->write != NULL) {
    char ch = (char)c;
    stdout->write(stdout->fd, &ch, 1);
  }

  return c;
}

int puts(const char *s) {
  if (stdout == NULL || stdout->write == NULL || s == NULL)
    return -1;

  while (*s != 0) {
    if (putchar((int)*s) < 0)
      return -1;
    ++s;
  }

  if (putchar('\n') < 0)
    return -1;
  return 0;
}

int getchar(void) {
  if (stdin == NULL || stdin->buffer == NULL)
    return -1;

  if (stdin->read_pos >= stdin->buffer_pos) {
    /* Blocking read */
    return -1;
  }

  return (int)stdin->buffer[stdin->read_pos++];
}

int getline(char **lineptr, size_t *n, FILE *stream) {
  if (lineptr == NULL || stream == NULL || n == NULL || stream->buffer == NULL)
    return -1;

  if (*lineptr == NULL) {
    *lineptr = malloc(*n);
    if (*lineptr == NULL) {
      return -1;
    }
  }

  char *raw_8 = *lineptr; // Write ptr
  char ch;                // Read char
  size_t read_size = 0;   // Size of line

  stream->read_pos = 0;
  stream->buffer_pos = 0;
  memset(stream->buffer, 0, stream->buffer_size);
  do {
    while (stream->read_pos >= stream->buffer_pos) {
      asm volatile("hlt");
    }

    if (read_size + 2 >= *n) {
      size_t new_n = *n << 1;
      char *new = malloc(new_n);
      if (new == NULL) {
        return read_size;
      }

      // Move char array
      memcpy((void *)new, (const void *)(*lineptr), read_size);
      free(*lineptr);
      *n = new_n;
      *lineptr = new;
    }

    raw_8 = (char *)((size_t)(*lineptr) + read_size);
    ch = stream->buffer[stream->read_pos++];

    switch (ch) {
    case '\b': {
      // *raw_8 = 0;
      if (read_size > 0)
        --read_size;
      break;
    }
    default: {
      *raw_8 = ch;
      ++read_size;
      break;
    }
    }

    if (ch == '\n') {
      raw_8 = (char *)((size_t)(*lineptr) + read_size);
      ch = 0;
      *raw_8 = 0;
      ++read_size;
    }
  } while (ch != 0);

  return read_size - 1;
}

FILE *fopen(const char *pathname, const char *mode);
void fclose(FILE *stream);

int fopendev(FILE *stream) {
  if (stream == NULL)
    return -1;

  for (int i = 0; i < MAX_FILE_DESCRIPTORS; ++i) {
    if (_file_descriptors[i] == NULL) {
      stream->fd = i;
      _file_descriptors[i] = stream;
      return i;
    }
  }
  return -1;
}

/* PRINTF */
uint32_t putint(int32_t x) {
  char buff[12];
  uint32_t i = 0;
  uint32_t wr = 0;

  // Negative
  if (x < 0) {
    putchar('-');
    ++wr;
    x = -x;
  }

  // Zero
  if (x == 0) {
    putchar('0');
    return 1;
  }

  // Calc digits
  while (x > 0) {
    buff[i++] = (x % 10) + '0';
    x /= 10;
  }

  // Write digits
  while (i > 0) {
    putchar(buff[--i]);
    ++wr;
  }

  return wr;
}

uint32_t puthex(int32_t x) {
  char buff[12];
  uint32_t i = 0;
  uint32_t wr = 0;

  // Negative
  if (x < 0) {
    putchar('-');
    ++wr;
    x = -x;
  }

  // Zero
  if (x == 0) {
    putchar('0');
    return 1;
  }

  // Calc hex digits
  while (x > 0) {
    uint8_t rem = x & 0x0F;
    buff[i++] = (rem < 10) ? (rem + '0') : (rem - 10 + 'A');
    x >>= 4;
  }

  // Write digits
  while (i > 0) {
    putchar(buff[--i]);
    ++wr;
  }

  return wr;
}

uint32_t putstr(const char *s) {
  uint32_t wr;
  while (*s != 0) {
    putchar(*s);
    ++s;
    ++wr;
  }

  return wr;
}

int printf(const char *format, ...) {
  va_list args;
  va_start(args, format);

  int written = 0;

  while (*format != 0) {
    if (*format == '%') {
      ++format;

      switch (*format) {
      case 'c': {
        char c = (char)va_arg(args, uint32_t);
        putchar(c);
        ++written;
        break;
      }

      case 's': {
        char *s = va_arg(args, char *);
        written += putstr(s);
        break;
      }

      case 'x': {
        unsigned int x = (unsigned int)va_arg(args, uint32_t);
        written += puthex(x);
        break;
      }

      case 'd': {
        unsigned int d = (unsigned int)va_arg(args, uint32_t);
        written += putint(d);
        break;
      }

      case '%': {
        putchar('%');
        ++written;
        break;
      }

      default: {
        putchar('%');
        putchar(*format);
        written += 2;
        break;
      }
      }
    } else {
      putchar(*format);
      ++written;
    }
    ++format;
  }

  va_end(args);
  return written;
}
