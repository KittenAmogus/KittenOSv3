#ifndef _DISK_H
#define _DISK_H

#include <stddef.h>
#include <stdint.h>

#define MAX_DISKS 256
#define BLOCK_SIZE 512

struct blk_dev;

typedef int (*blk_read_t)(struct blk_dev *dev, uint32_t sector, void *buffer,
                          size_t count);
typedef int (*blk_write_t)(struct blk_dev *dev, uint32_t sector,
                           const void *buffer, size_t count);

typedef struct blk_dev {
  uint32_t id;
  uint32_t total_sectors;
  uint32_t fs_offset;
  blk_read_t read_sector;
  blk_write_t write_sector;
} blk_dev_t;

#endif // _DISK_H
