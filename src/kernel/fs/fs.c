#include "fs.h"

#include "stdlib.h"

#include "drivers/ata/ata.h"

// Mounted disk copied data
superblock_t *sblock_mnt;

static uint32_t create_superblock(void) {
  // Alloc superblock in RAM
  superblock_t *sblock = malloc(BLOCK_SIZE);
  uint16_t *raw = (uint16_t*)sblock;
  if (sblock == NULL) return 1; // Not enough RAM

  // Identify disk
  if (ata_identify_disk(raw) != 0) return 2; // No disk

  uint32_t total_sectors;
  total_sectors = ((uint32_t)raw[61]) << 16;
  total_sectors += (uint32_t)raw[60];

  if (total_sectors < 128) return 3;  // Too small

  // Create superblock in RAM
  sblock->magic_start = SUPERBLOCK_MAGIC_START;
  sblock->block_size = BLOCK_SIZE;

  // Available
  sblock->inode_cnt = (total_sectors / 16);
  sblock->inode_cnt = \
    (sblock->inode_cnt > 4096 ? 4096 : sblock->inode_cnt);
  sblock->block_cnt = total_sectors;

  // Free
  sblock->inode_free_cnt = sblock->inode_cnt - 1; // First inode
  sblock->block_free_cnt = sblock->block_cnt - 3;
  // First data, first inode and superblock blocks
 
  // Tables
  sblock->inode_table_start = INODE_TABLE_START;
  sblock->data_table_start  = DATA_TABLE_START;

  // Validate block
  sblock->magic_end =   SUPERBLOCK_MAGIC_END;

  // Write sblock to disk
  ata_write_sector(1, (uint16_t*)sblock);
  free(sblock);
  return 0;
}


static uint32_t create_inode_table(void) {
  inode_t *first_inode = malloc(BLOCK_SIZE);
  if (first_inode == NULL) return 1;

  first_inode->inode_id = 0;  // First in table
  first_inode->size     = 0;  // No files yet
  first_inode->type     = FS_FILE_DIR;

  first_inode->addr_extend = 0; // No extended block
  first_inode->data_blocks[0] = DATA_TABLE_START;

  for (uint32_t i=1; i<DATA_CNT; ++i) {
    first_inode->data_blocks[i] = 0;  // Not so large
  }

  // Write inode
  ata_write_sector(INODE_TABLE_START, (uint16_t*)first_inode);
  free(first_inode);
  return 0;
}


static uint32_t create_data_table(void) {
  void *buff = malloc(BLOCK_SIZE);
  if (buff == NULL) return 1;

  // memset analog ))
  uint8_t *raw = (uint8_t*)buff;
  for (uint32_t i=0; i<BLOCK_SIZE; ++i) {
    raw[i] = 0;
  }

  dirent_t *dot = (dirent_t*)raw;
  dot->inode = 0;
  dot->type = FS_FILE_DIR;
  dot->name_len = 1;
  dot->rec_len  = 12; // Aligned
  dot->name[0] = '.'; // "."
  dot->name[1] = 0;

  dirent_t *dotdot = (dirent_t*)(raw+12);
  dotdot->inode = 0;
  dotdot->type = FS_FILE_DIR;
  dotdot->name_len = 2;
  dotdot->rec_len     = BLOCK_SIZE - 12;
  dotdot->name[0] = '.'; // ".."
  dotdot->name[1] = '.';
  dotdot->name[2] = 0;


  ata_write_sector(DATA_TABLE_START, (uint16_t*)buff);
  free(buff);
  return 0;
}


uint32_t mkfs(void) {
  uint32_t status;
 
  // Superblock
  status = create_superblock();
  if (status != 0) return status + 0;

  // First inode (inode table)
  status = create_inode_table();
  if (status != 0) return status + 10;

  // First inode data
  status = create_data_table();
  if (status != 0) return status + 20;
  return 0;
}

uint32_t mount(void) {
  void *buff = malloc(BLOCK_SIZE);
  if (buff == NULL) return 2;

  // Check if disk exists
  uint32_t status = ata_identify_disk((uint16_t*)buff);
  if (status != 0) {
    free(buff);
    return status;
  }

  // Read superblock
  ata_read_sector(1, (uint16_t*)buff);

  superblock_t *sb = (superblock_t*)buff;
  if (sb->magic_start != SUPERBLOCK_MAGIC_START) {
    free(buff);
    return 3;
  }
  if (sb->magic_end != SUPERBLOCK_MAGIC_END) {
    free(buff);
    return 4;
  }

  sblock_mnt = malloc(sizeof(superblock_t));
  if (sblock_mnt == NULL) return 5;

  // Superblock pointers
  uint8_t *raw = (uint8_t*)buff;
  uint8_t *rawd = (uint8_t*)sblock_mnt;

  // Copy superblock
  for (uint32_t i=0; i<BLOCK_SIZE; ++i) {
    *rawd = *raw;
    ++raw;
    ++rawd;
  }

  free(buff);
  return 0;
}

