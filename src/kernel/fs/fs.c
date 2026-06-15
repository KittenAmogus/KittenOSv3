#include "fs.h"

#include "errno.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"

#include "drivers/ata/ata.h"

superblock_t *mounted_sb;


// Static
static uint32_t mkfs_superblock(void) {
  // Allocate memory
  void *buffer = malloc(SIZE_BLOCK);
  if (buffer == NULL) return ENOMEM;

  // Pointers
  superblock_t *block_ptr = (superblock_t*)buffer; uint16_t *raw_16        = (uint16_t*)buffer;

  // Variables
  uint32_t sectors;

  // Get disk size
  if (ata_identify_disk(raw_16) != 0) {
    free(buffer);
    return ENODEV;
  }
  sectors = ((uint32_t)raw_16[61] << 16) | (uint32_t)raw_16[60];

  // == Fill buffer ==
  block_ptr->magic_start = MAGIC_SUPER_START;
  block_ptr->block_size  = SIZE_BLOCK;
 
  block_ptr->block_count = sectors;
  block_ptr->inodes_count = \
    MIN(sectors >> 4, MAX_INODES_ON_DISK);

  block_ptr->inode_table_start = SECTOR_INODE_TABLE;
  block_ptr->data_table_start  = SECTOR_DATA_TABLE(block_ptr->inodes_count);

  block_ptr->free_inodes = block_ptr->inodes_count - 1; // Root
  block_ptr->free_blocks = \
    block_ptr->block_count - block_ptr->data_table_start - 1;

  memset(block_ptr->_padding, 0, sizeof(block_ptr->_padding));
  block_ptr->magic_end = MAGIC_SUPER_END;

  // Write buffer
  ata_write_sector(SECTOR_SUPERBLOCK, raw_16);

  free(buffer);
  return SUCCESS;
}

static uint32_t mkfs_inode_table(void) {
  // Allocate memory
  void *buffer = malloc(SIZE_BLOCK);
  if (buffer == NULL) return ENOMEM;

  // Pointers
  uint16_t *raw_16        = (uint16_t*)buffer;
  inode_t *inode;

  ata_read_sector(SECTOR_SUPERBLOCK, raw_16);
  uint32_t data_table_start = ((superblock_t*)buffer)->data_table_start;

  // Reset buffer
  memset(buffer, 0, SIZE_BLOCK);

  // Fill inode
  inode = (inode_t*)buffer;
  inode->id   = 0;
  inode->size = 48; // Two aligned dirent_t with name length total of 5
  inode->type = FS_FILE_DIR;
  inode->data_blocks[0] = data_table_start;

  ata_write_sector(SECTOR_INODE_TABLE, raw_16);

  // Reset all inode table
  uint32_t block = SECTOR_INODE_TABLE + 1;
  memset(buffer, 0, SIZE_BLOCK);
  do {
    ata_write_sector(block, raw_16);
    ++block;
  } while (block < data_table_start);

  free(buffer);
  return SUCCESS;
}

static uint32_t mkfs_root_directory(void) {
  // Allocate memory
  void *buffer = malloc(SIZE_BLOCK);
  if (buffer == NULL) return ENOMEM;

  // Pointers
  uint16_t *raw_16  = (uint16_t*)buffer;
  dirent_t *dir     = (dirent_t*)buffer;

  // Get data table start
  ata_read_sector(SECTOR_SUPERBLOCK, raw_16);
  uint32_t data_table_start = ((superblock_t*)buffer)->data_table_start;
  memset(buffer, 0, SIZE_BLOCK);

  // Create 2 root links: ".", ".."

  // Dot
  dir->rec_len  = 24; // Aligned
  dir->inode_id = 0;
  dir->name_len = 2;  // "."
  dir->hash32   = 0;  // No hash now
  dir->type     = FS_FILE_DIR;
  memcpy(dir->name, ".", 2);
  dir = (dirent_t*)((uint8_t*)dir + 24);

  // Dot2
  dir->rec_len  = 24; // Aligned
  dir->inode_id = 0;
  dir->name_len = 3;  // ".."
  dir->type     = FS_FILE_DIR;
  dir->hash32   = 0;  // No hash now
  memcpy(dir->name, "..", 3);

  // Write
  ata_write_sector(data_table_start, raw_16);
  free(buffer);
  return SUCCESS;
}


// Global
uint32_t fs_makefs(void) {
  uint32_t status;

  status = mkfs_superblock();
  if (status != SUCCESS) return status;

  status = mkfs_inode_table();
  if (status != SUCCESS) return status;

  status = mkfs_root_directory();
  return status;
}

uint32_t fs_mountfs(void) {
  // Allocate memory
  void *buffer = malloc(SIZE_BLOCK);
  if (buffer == NULL) return ENOMEM;

  // Allocate mounted_sb
  if (mounted_sb == NULL) {
    mounted_sb = malloc(SIZE_BLOCK);
    if (mounted_sb == NULL) {
      free(buffer);
      return ENOMEM;
    }
  }

  // Pointers
  uint16_t *raw_16        = (uint16_t*)buffer;
  superblock_t *block_ptr = (superblock_t*)buffer;

  // Ensure disk exists
  if (ata_identify_disk(raw_16) != 0) {
    free(buffer);
    return ENODEV;
  }

  // Validate magics
  ata_read_sector(SECTOR_SUPERBLOCK, raw_16);
  if (block_ptr->magic_start != MAGIC_SUPER_START ||
      block_ptr->magic_end != MAGIC_SUPER_END) {
    free(buffer);
    free(mounted_sb);
    mounted_sb = NULL;
    return EINVAL;
  }

  // Copy superblock into ram
  memcpy(mounted_sb, raw_16, SIZE_BLOCK);

  return SUCCESS;
}

uint32_t fs_makedir(const char *path);

uint32_t fs_makefile(const char *path);



