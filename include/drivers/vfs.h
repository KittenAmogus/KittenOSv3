#ifndef _VFS_H
#define _VFS_H

#include <drivers/dev.h>
#include <stdint.h>

typedef enum {
  VFS_DIRENT_UNKNOWN = 0,
  VFS_DIRENT_FILE = 1,
  VFS_DIRENT_DIRECTORY = 2,
} vfs_dirent_type_e;

/* Structs for global functions */
struct fs_driver;
struct vfs_dirent;
struct vfs_driver;
struct vfs_mount;
struct vfs_device;

/* System limits */
#define VFS_MAX_DRIVERS 256
#define VFS_MAX_MOUNTED 256
#define VFS_MAX_DEVICES 256

/* System registered arrays */
extern struct vfs_driver *vfs_drivers[VFS_MAX_DRIVERS];
extern struct vfs_mount *vfs_mounted[VFS_MAX_MOUNTED];
extern struct vfs_device *vfs_devices[VFS_MAX_DEVICES];

/* Array filled positions */
extern int vfs_drivers_pos;
extern int vfs_mounted_pos;
extern int vfs_devices_pos;

/* === Functions === */
/* Register functions */
int vfs_register_driver(struct fs_driver *driver);
int vfs_unregister_driver(const char *driver_name);
int vfs_register_device(struct blk_dev *device, char **devname);

/* Device functions */
int vfs_mount_device(const char *device_path, const char *mountpoint,
                     const char *driver_name);
int vfs_makefs_device(const char *device_path, const char *driver_name);
int vfs_umount_device(const char *path);

/* File functions */
int vfs_creat(const char *path);
int vfs_mkdir(const char *path);
int vfs_rmdir(const char *path);
int vfs_unlink(const char *path);

/* === Function types === */
/* FS driver functions (device) */
typedef int (*fs_makefs_t)(struct blk_dev *dev);
typedef int (*fs_mountfs_t)(struct blk_dev *dev);
typedef int (*fs_umountfs_t)(struct blk_dev *dev);

/* FS driver functions (directory) */
typedef void *(*fs_opendir_t)(struct blk_dev *dev, const char *path);
typedef int (*fs_readdir_t)(void *handle, struct vfs_dirent *dest);
typedef void (*fs_closedir_t)(void *handle);

/* FS driver functions (file) */
typedef int (*fs_creat_t)(struct blk_dev *dev, const char *cleanpath,
                          int dirpath_len);
typedef int (*fs_mkdir_t)(struct blk_dev *dev, const char *cleanpath,
                          int dirpath_len);
typedef int (*fs_rmdir_t)(struct blk_dev *dev, const char *cleanpath,
                          int dirpath_len);
typedef int (*fs_unlink_t)(struct blk_dev *dev, const char *cleanpath,
                           int dirpath_len);

/* === Structs === */

typedef struct vfs_dirent {
  uint32_t size;
  uint32_t first_cluster;
  uint8_t type;
  char name[256];
} vfs_dirent_t;

typedef struct fs_driver {
  const char *name;

  fs_makefs_t makefs;
  fs_mountfs_t mountfs;
  fs_umountfs_t umountfs;

  fs_opendir_t opendir;
  fs_readdir_t readdir;
  fs_closedir_t closedir;

  fs_creat_t creat;
  fs_mkdir_t mkdir;
  fs_rmdir_t rmdir;
  fs_unlink_t unlink;
} fs_driver_t;

typedef struct vfs_driver {
  fs_driver_t *driver;
  void *handle;
} vfs_driver_t;

typedef struct vfs_mount {
  char *mountpoint;
  blk_dev_t *dev;
  fs_driver_t *driver;
  void *fs_instance;
} vfs_mount_t;

typedef struct vfs_device {
  char *devpath;
  blk_dev_t *dev;
} vfs_device_t;

#endif // _VFS_H
