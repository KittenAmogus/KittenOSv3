#include "fs.h"

#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"

#include "drivers/ata/ata.h"

// Mounted disk copied data
superblock_t *superblock_mounted;

static uint32_t create_superblock(void) {
  // Allocate buffer
  void *buffer = malloc(BLOCK_SIZE);
  if (buffer == NULL) return ENOMEM;
  memset(buffer, 0, BLOCK_SIZE);

  // Variables
  uint32_t sectors;
  uint16_t *raw = (uint16_t *)buffer;

  // Identify disk
  uint32_t status = ata_identify_disk(raw);
  if (status != SUCCESS) return status;

  // Count sectors
  sectors = (((uint32_t)raw[61]) << 16) | ((uint32_t)raw[60]);
  if (sectors < MIN_SECTORS) return ENOSPC;

  // Create superblock in buffer
  superblock_t *block_ptr = (superblock_t*)buffer;

  // Write data
  block_ptr->magic_start  = SUPERBLOCK_MAGIC_START;
  block_ptr->block_size   = BLOCK_SIZE;
  block_ptr->inode_cnt    = MIN(MAX_INODES, sectors >> 4);
  block_ptr->block_cnt    = sectors;
  block_ptr->inode_free_cnt = block_ptr->inode_cnt - 1;
  block_ptr->block_free_cnt = block_ptr->block_cnt - 3;
  block_ptr->inode_table_start  = INODE_TABLE_START;
  block_ptr->data_table_start   = DATA_TABLE_START;
  block_ptr->magic_end          = SUPERBLOCK_MAGIC_END;

  // Write to disk
  raw = (uint16_t*)buffer;
  ata_write_sector(1, raw); // 1 - superblock
  free(buffer); // Free block RAM
  return SUCCESS;
}

static uint32_t create_inode_table(void) {
  // Allocate buffer
  void *buffer = malloc(BLOCK_SIZE);
  if (buffer == NULL) return ENOMEM;
  memset(buffer, 0, BLOCK_SIZE);

  // Variables
  uint16_t *raw = (uint16_t*)buffer;

  // Create inode in buffer
  inode_t *inode_ptr = (inode_t*)buffer;

  // Write data
  inode_ptr->inode_id = INODE_TABLE_START;
  inode_ptr->size     = 0;
  inode_ptr->type     = FS_FILE_DIR;
  inode_ptr->addr_extend    = 0;
  memset(inode_ptr->data_blocks, 0, sizeof(uint32_t) * DATA_CNT);
  inode_ptr->data_blocks[0] = DATA_TABLE_START;

  // Write to disk
  ata_write_sector(INODE_TABLE_START, raw);
  free(buffer);
  return SUCCESS;
}

static uint32_t create_data_table(void) {
  // Allocate buffer
  void *buffer = malloc(BLOCK_SIZE);
  if (buffer == NULL) return ENOMEM;
  memset(buffer, 0, BLOCK_SIZE);

  // Variables
  uint16_t *raw = (uint16_t*)buffer;
  dirent_t *dot;

  // Write data
  dot = (dirent_t*)buffer;
  dot->inode  = INODE_TABLE_START;
  dot->type   = FS_FILE_DIR;
  dot->name_len = 1;
  dot->rec_len  = 12; // Aligned
  memcpy(dot->name, ".", 2);  // Copy name

  // Next file
  dot = (dirent_t*)((char*)dot + 12);
  dot->inode  = INODE_TABLE_START;
  dot->type   = FS_FILE_DIR;
  dot->name_len = 2;
  dot->rec_len  = BLOCK_SIZE - 12;  // Aligned
  memcpy(dot->name, "..", 3);       // Copy name

  // Write do disk
  ata_write_sector(DATA_TABLE_START, raw);
  free(buffer);
  return SUCCESS;
}

uint32_t mkfs(void) {
  uint32_t status;
 
  // Superblock
  status = create_superblock();
  if (status != SUCCESS) return status;

  // First inode (inode table)
  status = create_inode_table();
  if (status != SUCCESS) return status;

  // First inode data
  status = create_data_table();
  if (status != SUCCESS) return status;
  return SUCCESS;
}

uint32_t mount(void) {
  // Allocate buffer
  void *buffer = malloc(BLOCK_SIZE);
  if (buffer == NULL) return ENOMEM;

  // Variables
  uint16_t *raw = (uint16_t*)buffer;
  uint32_t status;

  // Identify
  status = ata_identify_disk(raw);
  if (status != SUCCESS) {
    free(buffer);
    return status;
  }

  // Read superblock (1)
  ata_read_sector(1, raw);

  // Validate
  superblock_t *block_ptr = (superblock_t*)buffer;
  if (block_ptr->magic_start != SUPERBLOCK_MAGIC_START ||
    block_ptr->magic_end != SUPERBLOCK_MAGIC_END) {
    free(buffer);
    return EINVAL;
  }

  // Create mount buffer
  if (superblock_mounted == NULL)
    superblock_mounted = malloc(BLOCK_SIZE);
  if (superblock_mounted == NULL) return ENOMEM;

  // Copy superblock
  memcpy(superblock_mounted, buffer, BLOCK_SIZE);
  free(buffer);
  return SUCCESS;
}

