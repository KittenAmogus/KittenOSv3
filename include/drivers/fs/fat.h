#ifndef _FS_FAT_H
#define _FS_FAT_H

#include <stddef.h>
#include <stdint.h>

#include <drivers/disk.h>

int fat_makefs(blk_dev_t *dev);
int fat_mountfs(blk_dev_t *dev);
int fat_umountfs(blk_dev_t *dev);

#endif // _FS_FAT_H
