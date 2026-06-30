#include <drivers/fs/fat.h>

int fat_makefs(blk_dev_t *dev);
int fat_mountfs(blk_dev_t *dev);
int fat_umountfs(blk_dev_t *dev);
