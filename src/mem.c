#include <stdlib.h>

#define BLOCK_MIN_SIZE  4
#define BLOCK_MAGIC     0xABCDEF01

typedef struct mem_block {
  struct mem_block *prev;
  struct mem_block *next;
  uint32_t magic;
  size_t size;

  struct {
    uint32_t is_free  : 1;
    uint32_t _pad     : 31;
  } __attribute__((packed));

} __attribute__((packed)) mem_block_t;

#define META_SIZE sizeof(mem_block_t)


static uint8_t *_heap_start = NULL;
static uint8_t *_heap_max   = NULL;

static mem_block_t *_first_block  = NULL;

void k_mem_init(size_t max) {
  // Create heap
  _heap_start = _kernel_end;
  _heap_max = max;

  // Create first block
  _first_block = (mem_block_t*)_heap_start;
  _first_block->magic = BLOCK_MAGIC;
  _first_block->size = _heap_max - _heap_start - META_SIZE;
  _first_block->prev = NULL;
  _first_block->next = NULL;
}

void *malloc(size_t size) {
  if (size < 1) {
    return NULL;
  }
  size = (size + 3) & ~3;
  mem_block_t *block = _first_block;

  while (block != NULL && block->magic == BLOCK_MAGIC) {
    // Available and enough size
    if (block->is_free && block->size >= size) {
      block->is_free = 0;

      // Can fit another block
      if (block->size >= size + (META_SIZE + BLOCK_MIN_SIZE)) {

        // Create new block
        mem_block_t *new = (mem_block_t*)((uint8_t*)block + META_SIZE + size);
        new->magic = BLOCK_MAGIC;
        new->size = (block->size - META_SIZE - size);
        new->prev = block;
        new->next = block->next;

        // Resize old block
        block->next = new;
        block->size = size;
      }

      // Data addr
      return (void*)((char*)block + META_SIZE);
    }

    // Next block
    block = block->next;
  }

  // Can't allocate <size> bytes
  return NULL;
}

void free(void *ptr) {
  if (ptr < _heap_start + META_SIZE || ptr == NULL || ptr >= _heap_max)
    return;

  mem_block_t *block = (mem_block_t*)((uint8_t*)ptr - META_SIZE);
  if (block->magic != BLOCK_MAGIC) return;  // Pointer not allocated

  // Free block
  block->magic = ~BLOCK_MAGIC;
  block->is_free = 1;

  mem_block_t *prev = block->prev;
  mem_block_t *next = block->next;

  // Update right
  if (next != NULL && next->is_free && next->magic == BLOCK_MAGIC) {
    block->size += META_SIZE + next->size;
    block->next = next->next;
    if (block->next != NULL) {
      block->next->prev = block;
    }
    next->magic = ~BLOCK_MAGIC;
  }

  // Update left
  if (prev != NULL && prev->is_free && prev->magic == BLOCK_MAGIC) {
    prev->size += META_SIZE + block->size;
    prev->next = block->next;
    if (block->next != NULL) {
      block->next->prev = prev;
    }
    block->magic = ~BLOCK_MAGIC;
  }
}

void *calloc(size_t nmemb, size_t size) {
  void *buffer = malloc(nmemb * size);
  if (buffer != NULL) {
    memset(buffer, 0, nmemb * size);
  }
  return buffer;
}

void *realloc(void *ptr, size_t size) {
  if (ptr == NULL) {
    return malloc(size);
  } else if (size == 0) {
    free(ptr);
    return NULL;
  }

  mem_block_t *block = (mem_block_t*)((uint8_t*)ptr - META_SIZE);
  if (block <= _heap_start) return NULL;

  if (block->size >= size + (META_SIZE + BLOCK_MIN_SIZE)) {
    mem_block_t *new = \
      (mem_block_t*)((uint8_t*)ptr + size);

    /* Create new block */
    new->magic = BLOCK_MAGIC;
    new->size = (block->size - size) - META_SIZE;
    new->prev = block;
    new->next = block->next;
    new->is_free = 1;

    if (block->next != NULL) {
      block->next->prev = new;
    }
    block->next = new;

    block->size = size;
    return ptr;
  }

  void *newptr = malloc(size);
  if (newptr == 0) {
    free(ptr);
    return NULL;
  }

  memcpy(newptr, ptr, block->size);
  free(ptr);
  return newptr;
}

