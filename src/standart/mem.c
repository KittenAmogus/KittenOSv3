#include <stdlib.h>

extern char _heap_start;
static mem_block_t *_first_block = NULL;
static void *_heap_end = NULL;

void k_mem_init(size_t mem_size) {
  _heap_end = (void *)((size_t)&_heap_start + mem_size);

  // Create first memory block
  _first_block = (mem_block_t *)&_heap_start;
  _first_block->magic = MEM_BLOCK_MAGIC;
  _first_block->prev = NULL;
  _first_block->next = NULL;
  // _first_block->size = mem_size - META_SIZE - ((size_t));
  _first_block->size = ((size_t)_heap_end - (size_t)_first_block) - META_SIZE;
  _first_block->is_free = 1;
}

void *malloc(size_t size) {
  if (size < 1) {
    return NULL;
  }
  size = (size + 3) & ~3;
  mem_block_t *block = _first_block;

  while (block != NULL && block->magic == MEM_BLOCK_MAGIC) {
    // Available and enough size
    if (block->is_free && block->size >= size) {

      // Not enough RAM
      if ((size_t)block + size >= (size_t)_heap_end)
        return NULL;

      block->is_free = 0;

      // Can fit another block
      if (block->size >= size + (META_SIZE)) {

        // Create new block
        mem_block_t *new = (mem_block_t *)((uint8_t *)block + META_SIZE + size);
        new->magic = MEM_BLOCK_MAGIC;
        new->size = (block->size - META_SIZE - size);
        new->prev = block;
        new->next = block->next;
        new->is_free = 1;
        new->_padding = 0x0FFFFFFF;

        if (new->next != NULL) {
          new->next->prev = new;
        }

        // Resize old block
        block->next = new;
        block->size = size;
      }

      // Data addr
      return (void *)((char *)block + META_SIZE);
    }

    // Next block
    block = block->next;
  }

  // Can't allocate <size> bytes
  return NULL;
}

void free(void *ptr) {
  if ((size_t)ptr < (size_t)_first_block + META_SIZE || ptr == NULL ||
      (size_t)ptr >= (size_t)_heap_end)
    return;

  mem_block_t *block = (mem_block_t *)((size_t)ptr - META_SIZE);
  if (block->magic != MEM_BLOCK_MAGIC || block->is_free)
    return; // Pointer not allocated

  // Free block
  block->is_free = 1;

  // Next
  if (block->next != NULL && block->next->is_free &&
      block->next->magic == MEM_BLOCK_MAGIC) {
    mem_block_t *next_block = block->next;
    block->size += META_SIZE + next_block->size;
    block->next = next_block->next;
    if (block->next != NULL) {
      block->next->prev = block;
    }
    next_block->magic = ~MEM_BLOCK_MAGIC;
  }

  // Prev
  if (block->prev != NULL && block->prev->is_free &&
      block->prev->magic == MEM_BLOCK_MAGIC) {
    mem_block_t *prev_block = block->prev;
    prev_block->size += META_SIZE + block->size;
    prev_block->next = block->next;
    if (block->next != NULL) {
      block->next->prev = prev_block;
    }
    block->magic = ~MEM_BLOCK_MAGIC;
  }
}
