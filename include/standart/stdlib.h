#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>
#include <stdint.h>

// #define MEM_BLOCK_MAGIC 0xCA75C0DE
#define MEM_BLOCK_MAGIC 0xAABBCCDD

typedef struct mem_block {
  struct mem_block *prev;
  struct mem_block *next;
  uint32_t magic;
  size_t size;

  struct {
    uint32_t is_free : 1;
    uint32_t _padding : 31;
  } __attribute__((packed));
} mem_block_t;
#define META_SIZE (sizeof(mem_block_t))

typedef struct {
  int quot;
  int rem;
} div_t;

void k_mem_init(size_t mem_size);

void *malloc(size_t size);
void free(void *ptr);

uint32_t atoi(const char *s);
int32_t strtol(const char *nptr, char **endptr, int base);
uint32_t strtoul(const char *nptr, char **endptr, int base);

void exit(uint32_t code) __attribute__((noreturn));
void abort(void) __attribute__((noreturn));

uint32_t abs(int32_t i);
uint64_t labs(int64_t l);
div_t div(int numer, int denom);

uint32_t rand(void);
void srand(uint32_t seed);

#endif // _STDLIB_H
