#ifndef _RAMDISK_H
#define _RAMDISK_H

#include <drivers/disk.h>

#define RD_DISK_PREFIX 0x1000

blk_dev_t *rd_createdev(size_t size);

#endif // _RAMDISK_H
