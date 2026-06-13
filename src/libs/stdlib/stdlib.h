#ifndef STDLIB_H
#define STDLIB_H

#include "stdint.h"

#define NULL ((void*)0)
typedef uint32_t size_t;

typedef struct Block_s {
  uint32_t magic;
  size_t size;
  uint8_t isFree;
  struct Block_s *next;
} Block;

#define META_SIZE sizeof(Block)
#define BLOCK_MAGIC 0xDEADC0DE

// Do not split if second block->size will be smaller than X bytes
#define MIN_BLOCK_SIZE  8

void initHeap(size_t ramSize);
void *malloc(size_t size);
void *calloc(size_t n, size_t size);
void *realloc(void *p, size_t size);
void free(void *p);

size_t get_heap_free(void);
size_t get_heap_size(void);

#endif // STDLIB_H

