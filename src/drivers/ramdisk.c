#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/disk.h>
#include <drivers/ramdisk.h>

static void *_rd_buffers[MAX_DISKS] = {NULL};

static int _rd_write_sector(struct blk_dev *dev, uint32_t sector,
                            const void *buffer, size_t count) {
  if (dev == NULL || buffer == NULL)
    return EINVAL;

  if (dev->id < RD_DISK_PREFIX || dev->id > (RD_DISK_PREFIX + MAX_DISKS))
    return EACCES;

  uint32_t dev_id = dev->id - RD_DISK_PREFIX;
  if (_rd_buffers[dev_id] == NULL)
    return ENODEV;

  void *sec_ptr = (void *)((size_t)_rd_buffers[dev_id] + (sector * BLOCK_SIZE));
  memcpy(sec_ptr, buffer, count * BLOCK_SIZE);

  return SUCCESS;
}

static int _rd_read_sector(blk_dev_t *dev, uint32_t sector, void *buffer,
                           size_t count) {
  if (dev == NULL || buffer == NULL)
    return EINVAL;

  if (dev->id < RD_DISK_PREFIX || dev->id > (RD_DISK_PREFIX + MAX_DISKS))
    return EACCES;

  uint32_t dev_id = dev->id - RD_DISK_PREFIX;
  if (_rd_buffers[dev_id] == NULL)
    return ENODEV;

  const void *sec_ptr =
      (const void *)((size_t)_rd_buffers[dev_id] + (sector * BLOCK_SIZE));
  memcpy(buffer, sec_ptr, count * BLOCK_SIZE);

  return SUCCESS;
}

blk_dev_t *rd_createdev(size_t size) {
  blk_dev_t *dev = malloc(sizeof(blk_dev_t));
  if (dev == NULL)
    return NULL;

  for (int i = 0; i < MAX_DISKS; ++i) {
    if (_rd_buffers[i] == NULL) {
      _rd_buffers[i] = malloc(size);
      if (_rd_buffers[i] == NULL) {
        free(dev);
        return NULL;
      }

      dev->total_sectors = (size / BLOCK_SIZE);
      dev->id = (RD_DISK_PREFIX + i);
      dev->read_sector = _rd_read_sector;
      dev->write_sector = _rd_write_sector;
      return dev;
    }
  }

  return NULL;
}
