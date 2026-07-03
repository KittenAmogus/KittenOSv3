#include <drivers/disk.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define FS_DRIVERS_MAX 128
static fs_driver_t *_fs_drivers[FS_DRIVERS_MAX];
static int _fs_drivers_pos = 0;

#define MAX_MOUNTPOINTS 256
vfs_mount_t *mounted_devices[MAX_MOUNTPOINTS];
int mounted_devices_pos = 0;

static int vfs_ld_driver(const char *driver, fs_driver_t **destptr) {
  if (driver == NULL || destptr == NULL)
    return EINVAL;

  fs_driver_t *dest;

  for (int i = 0; i < _fs_drivers_pos; ++i) {
    dest = _fs_drivers[i];
    if (strcmp(driver, dest->name) == 0) {
      *destptr = dest;
      return SUCCESS;
    }
  }
  *destptr = NULL;
  return ENOENT;
}

int vfs_register_driver(fs_driver_t *driver) {
  if (_fs_drivers_pos >= FS_DRIVERS_MAX)
    return ENOMEM;
  _fs_drivers[_fs_drivers_pos++] = driver;
  return SUCCESS;
}
void *vfs_opendir(blk_dev_t *dev, const char *path) {
  if (path == NULL)
    return NULL;

  const char *driver = "fat"; // TODO: FS driver

  vfs_drv_t *vdrv = malloc(sizeof(vfs_drv_t));
  if (vdrv == NULL)
    return NULL;

  int err = vfs_ld_driver(driver, &vdrv->driver);
  if (err != SUCCESS) {
    free(vdrv);
    return NULL;
  }

  vdrv->handle = vdrv->driver->opendir(dev, path);
  return vdrv;
}

int vfs_readdir(void *handle, dirent_t *dirent) {
  if (handle == NULL)
    return -1;

  vfs_drv_t *vdrv = (vfs_drv_t *)handle;
  if (vdrv->driver == NULL || vdrv->handle == NULL)
    return -1;

  return vdrv->driver->readdir(vdrv->handle, dirent);
}

void vfs_closedir(void *handle) {
  if (handle == NULL)
    return;

  vfs_drv_t *vdrv = (vfs_drv_t *)handle;
  if (vdrv->driver != NULL) {
    vdrv->driver->closedir(vdrv->handle);
  }
  free(vdrv);
}

int vfs_makefs(const char *path) {
  if (path == NULL)
    return EINVAL;

  const char *driver = "fat"; // TODO: FS driver

  fs_driver_t *drv;
  int err = vfs_ld_driver(driver, &drv);
  if (err != SUCCESS)
    return err;

  // TODO: Find device
  blk_dev_t *dev = (blk_dev_t *)path;
  err = drv->makefs(dev);

  return err;
}

int vfs_mountfs(const char *path, const char *mountpoint) {
  if (path == NULL || mountpoint == NULL)
    return EINVAL;

  const char *driver = "fat"; // TODO: FS driver

  fs_driver_t *drv;
  int err = vfs_ld_driver(driver, &drv);
  if (err != SUCCESS)
    return err;

  // TODO: Find device
  blk_dev_t *dev = (blk_dev_t *)path;

  if (mounted_devices_pos >= MAX_MOUNTPOINTS)
    return ENOMEM;

  err = drv->mountfs(dev);
  if (err != SUCCESS)
    return err;

  char *mountpoint_cpy = malloc(strlen(mountpoint) + 1);
  if (mountpoint_cpy == NULL)
    return ENOMEM;

  vfs_mount_t *mnt = malloc(sizeof(vfs_mount_t));
  if (mnt == NULL) {
    free(mountpoint_cpy);
    return ENOMEM;
  }

  memcpy(mountpoint_cpy, mountpoint, strlen(mountpoint) + 1);

  mnt->dev = dev;
  mnt->driver = drv;
  mnt->fs_instance = dev->fs_private;
  mnt->mountpoint = mountpoint_cpy;
  mounted_devices[mounted_devices_pos++] = mnt;

  return SUCCESS;
}

int vfs_umountfs(const char *path, const char *mountpoint) {
  if (path == NULL && mountpoint == NULL)
    return EINVAL;

  if (path == NULL) { // Umount by mountpoint
    for (int i = 0; i < mounted_devices_pos; ++i) {
      vfs_mount_t *mnt = mounted_devices[i];
      if (mnt == NULL || strcmp(mountpoint, mnt->mountpoint) != 0)
        continue;

      // printf("Compare '%s' to '%s': %d\n", mountpoint, mnt->mountpoint,
      //        strcmp(mountpoint, mnt->mountpoint) != 0);

      /* Unmount */
      mnt->driver->umountfs(mnt->dev);
      free(mnt->mountpoint);
      free(mnt);

      // Move last
      --mounted_devices_pos;
      mounted_devices[i] = mounted_devices[mounted_devices_pos];
      return SUCCESS;
    }

    return ENOENT;
  } else {
    return 0xAA55; // TODO: Umount by path
  }

  return EINVAL;
}

char *vfs_next_subd(char *path) {
  if (path == NULL)
    return NULL;

  /* lpathip slash */
  while (*path == '/' && *path != 0)
    ++path;

  /* Skip to first slash */
  while (*path != '/' && *path != 0)
    ++path;
  if (*path == 0)
    return NULL;

  /* Replace slash to '\0' */
  while (*path == '/' && *path != 0) {
    *path = 0;
    ++path;
  }

  /* End of pathing */
  if (*path == '/' || *path == 0)
    return NULL;

  return path;
}
