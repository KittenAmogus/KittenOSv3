#ifndef _DEV_H
#define _DEV_H

#include <stdint.h>

#define DEVICE_PREFIX "/dev/"

/* Device driver functions */
typedef int (*blk_read_t)(struct blk_dev *dev, uint32_t sector, void *buffer,
                          uint32_t count);
typedef int (*blk_write_t)(struct blk_dev *dev, uint32_t sector,
                           const void *buffer, uint32_t count);

typedef struct blk_dev {
  uint32_t id;
  uint32_t total_sectors;

  blk_read_t read_sector;
  blk_write_t write_sector;

  void *fs_private;
} blk_dev_t;

#endif // _DEV_H
