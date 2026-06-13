#include "stdlib.h"

extern size_t _end;
static Block * _firstBlock = NULL;

extern uint32_t _grub_ram_size;

void initHeap(size_t ramSize) {
  _firstBlock = (Block*) &_end;  // end -> end of kernel's RAM
  _firstBlock->size = (ramSize - (unsigned int)_firstBlock) - META_SIZE;
  _firstBlock->isFree = 1;
  _firstBlock->next = NULL;
  _firstBlock->magic = BLOCK_MAGIC;
}

void *malloc(size_t size) {
  size = (size + 3) & ~3; // Align to 4 bytes
  Block *bptr = _firstBlock;

  while (bptr != NULL) {
    // Block is free and large enough
    if (bptr->isFree && bptr->size >= size) {
      bptr->isFree = 0;

      // Block is large enough to be split
      if (bptr->size >= size + (META_SIZE + MIN_BLOCK_SIZE)) {

        // Create new block
        Block *new = (Block*)((char*)bptr + META_SIZE + size);  // Create META
        new->size = bptr->size - META_SIZE - size;              // Free size - META
        new->isFree = 1;        // Free block
        new->next = bptr->next; // Insert block
        new->magic = BLOCK_MAGIC;

        // Resize and insert
        bptr->size = size;
        bptr->next = new;
      }

      return (void*)((char*)bptr + META_SIZE);
    }

    // Next block in heap
    bptr = bptr->next;
  }

  return NULL;  // Heap not initialized
}

// TODO:
void *calloc(size_t n, size_t size) {
  return NULL;
}

// TODO:
void *realloc(void *p, size_t size) {
  return NULL;
}

void free(void *p) {
  // Not valid pointer
  if (p == NULL || (size_t)p <= META_SIZE) return;

  // Get block pointer
  Block *bptr = (Block*)((char*)p - META_SIZE);
  if (bptr->magic != BLOCK_MAGIC) return; // Invalid pointer

  bptr->isFree = 1;
  bptr->magic = ~BLOCK_MAGIC; // Double-free protection

  Block *next = bptr->next;

  // Join all next free blocks
  while (next != NULL && next->isFree) {
    bptr->size += (META_SIZE + next->size);
    bptr->magic = ~BLOCK_MAGIC;
    next = next->next;
  }
  bptr->next = next;
}

size_t get_heap_free(void) {
  Block *bptr = _firstBlock;
  size_t free_ram = 0;
  while (bptr != NULL) {
    if (bptr->isFree)
      free_ram += (bptr->size);
    bptr = bptr->next;
  }
  return free_ram;
}

size_t get_heap_size(void) {
  Block *bptr = _firstBlock;
  size_t ram = 0;
  while (bptr != NULL) {
    ram += (bptr->size);
    bptr = bptr->next;
  }
  return (ram > _grub_ram_size ? ram : _grub_ram_size);
}

