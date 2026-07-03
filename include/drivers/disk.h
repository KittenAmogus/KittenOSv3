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

typedef enum {
  DIRENT_UNKNOWN = 0,
  DIRENT_FILE,
  DIRENT_DIR,
} DirentType;

typedef struct blk_dev {
  uint32_t id;
  uint32_t total_sectors;
  uint32_t fs_offset;
  blk_read_t read_sector;
  blk_write_t write_sector;
  void *fs_private;
} blk_dev_t;

typedef struct dirent {
  uint32_t size;
  uint32_t first_cluster;
  uint8_t type;
  char name[256];
} dirent_t;

typedef void *(*open_dir_t)(blk_dev_t *dev, const char *path);
typedef int (*read_dir_t)(void *handle, dirent_t *dirent);
typedef void (*close_dir_t)(void *handle);

typedef int (*makefs_t)(blk_dev_t *dev);
typedef int (*mountfs_t)(blk_dev_t *dev);
typedef int (*umountfs_t)(blk_dev_t *dev);

typedef struct fs_driver {
  const char *name;

  open_dir_t opendir;
  read_dir_t readdir;
  close_dir_t closedir;

  makefs_t makefs;
  mountfs_t mountfs;
  umountfs_t umountfs;
} fs_driver_t;

typedef struct {
  fs_driver_t *driver;
  void *handle;
} vfs_drv_t;

typedef struct vfs_mount {
  char *mountpoint;
  blk_dev_t *dev;
  fs_driver_t *driver;
  void *fs_instance;
} vfs_mount_t;

typedef struct {
  char *devpath;
  blk_dev_t *dev;
} vfs_dev_t;

extern vfs_mount_t *mounted_devices[];
extern int mounted_devices_pos;

char *vfs_next_subd(char *path);
int vfs_register_driver(fs_driver_t *driver);
int vfs_register_device(blk_dev_t *device, const char *devname);

void *vfs_opendir(const char *path);
int vfs_readdir(void *handle, dirent_t *dirent);
void vfs_closedir(void *handle);

int vfs_makefs(const char *path, const char *driver);
int vfs_mountfs(const char *path, const char *mountpoint, const char *driver);
int vfs_umountfs(const char *path, const char *mountpoint);

int vfs_creat(const char *path);
int vfs_mkdir(const char *path);

#endif // _DISK_H
