#ifndef FS_H
#define FS_H

#include "stdint.h"

#define BLOCK_SIZE  512 // Disk sector size
#define MIN_SECTORS 128
#define MAX_INODES  4096
#define DATA_CNT    12  // Data block count for one file
#define SUPERBLOCK_MAGIC_START  0xCA75C0DE  // Start of superblock
#define SUPERBLOCK_MAGIC_END    0x0000AA55  // End of superblock

#define INODE_TABLE_START 2
#define DATA_TABLE_START  130

typedef enum {
  FS_FILE_UNDEF = 0,
  FS_FILE_FILE  = 1,
  FS_FILE_DIR   = 2
} FS_FILE_TYPE;

typedef struct __attribute__((packed)) {
  uint32_t  magic_start;
  uint32_t  block_size;

  uint32_t  inode_cnt;
  uint32_t  block_cnt;

  uint32_t  inode_free_cnt;
  uint32_t  block_free_cnt;

  uint32_t  inode_table_start;
  uint32_t  data_table_start;

  // stretch to sector size
  uint32_t _unused[128 - 8 - 1];

  uint32_t  magic_end;
} superblock_t; // 512 bytes

typedef  struct {
  uint32_t inode_id;    // Table index
  uint32_t size;        // File size (bytes)
  uint32_t type;        // FILE / DIR
  uint32_t addr_extend; // Block with addrs for large files

  uint32_t data_blocks[DATA_CNT];  // Data addrs
} inode_t;  // 16 + (4*DATA_CNT) bytes
#define INODE_SIZE  sizeof(inode_t)

typedef struct {
  uint32_t inode;   // Inode addr
  uint8_t type;     // File type
  uint8_t name_len; // Name len
  uint16_t rec_len; // Dirent name
  char name[];      // Name (1 - 255) + '\0'
} dirent_t; // 8 + (2-256)

uint32_t mkfs(void);
uint32_t mount(void);

uint32_t create_inode(inode_t *src);

#endif // FS_H

