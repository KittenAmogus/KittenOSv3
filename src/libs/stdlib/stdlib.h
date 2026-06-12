#ifndef STDLIB_H
#define STDLIB_H

#include "stdint.h"

#define NULL ((void*)0)
typedef uint32_t size_t;

typedef struct Block_s {
  size_t size;
  uint8_t isFree;
  struct Block_s *next;
} __attribute__((packed)) Block;

#define META_SIZE sizeof(Block)

// Do not split if second block->size will be smaller than X bytes
#define MIN_BLOCK_SIZE  8

void initHeap(size_t ramSize);
void *malloc(size_t size);
void *calloc(size_t n, size_t size);
void *realloc(void *p, size_t size);
void free(void *p);

#endif // STDLIB_H

