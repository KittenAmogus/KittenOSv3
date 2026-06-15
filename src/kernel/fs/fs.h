#ifndef FS_H
#define FS_H

#include "stdint.h"

#define DATA_BLOCK_CNT  12

typedef enum {
  FS_FILE_UNDEF = 0,
  FS_FILE_FILE  = 1,
  FS_FILE_DIR   = 2
} file_type;

typedef struct {
  uint32_t id;    // Sector id (LBA)
  uint32_t size;  // File size (bytes)
  uint32_t type;  // Enum file_type
  uint32_t ext_data;  // Block with data_blocks extension
  uint32_t data_blocks[DATA_BLOCK_CNT];
} inode_t;  // 64 bytes

typedef struct {
  uint32_t rec_len;   // Full structure size
  uint32_t inode_id;  // File inode id
  uint32_t hash32;    // Filename hash

  // Packed in 4 bytes to avoid extra 4bytes alignment
  struct {
    uint32_t name_len : 8;
    uint32_t type     : 8;
    uint32_t _padding : 16;
  } __attribute__((packed));

  char name[];  // 1-255 + '\0'
} dirent_t; // 8 + 2-256 bytes


#define MAX_INODES_IN_BLOCK (SIZE_BLOCK / SIZE_INODE)
#define MAX_INODES_ON_DISK  0x8000  // 32K inodes
#define MAX_FILENAME  255           // 0xFF
#define MAX_FILESIZE  0xFFFFFFFF    // 2^32 - 1, 4GB - 1b

#define SIZE_BLOCK  512
#define SIZE_INODE  (sizeof(inode_t))
#define SIZE_DIRENT_BASE  16  // (sizeof(uint32_t) * 4)
#define SIZE_INODE_TABLE(inodes)  (inodes / MAX_INODES_IN_BLOCK)

#define SECTOR_SUPERBLOCK   1
#define SECTOR_INODE_TABLE  2
#define SECTOR_DATA_TABLE(inodes) ((inodes / MAX_INODES_IN_BLOCK) + SECTOR_INODE_TABLE)

#define MAGIC_SUPER_START 0xDEC075CA  // CA75C0DE
#define MAGIC_SUPER_END   0xFECAADDE  // DEADCAFE

typedef struct {
  uint32_t magic_start;
  uint32_t block_size;

  // Total space
  uint32_t block_count;
  uint32_t inodes_count;

  // Free space
  uint32_t free_inodes;
  uint32_t free_blocks;

  // Start addrs
  uint32_t inode_table_start;
  uint32_t data_table_start;

  uint32_t _padding[(SIZE_BLOCK/4) - 8 - 1];

  uint32_t magic_end;
} superblock_t;


extern superblock_t *mounted_sb;

uint32_t fs_makefs(void);
uint32_t fs_mountfs(void);

uint32_t fs_makedir(const char *path);
uint32_t fs_makefile(const char *path);

#endif // FS_H

