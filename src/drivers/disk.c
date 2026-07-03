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

#define MAX_DEVICES 256
vfs_dev_t *_all_devices[MAX_DEVICES];
static int _all_devices_pos = 0;

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

static int vfs_dev_by_mountpoint(const char *path, blk_dev_t **dev) {
  if (dev == NULL)
    return EINVAL;

  blk_dev_t *lastdev;
  int lastlen = 0;

  for (int i = 0; i < mounted_devices_pos; ++i) {
    if (mounted_devices[i] == NULL)
      continue;

    /* Now not only exact mountpoint */
    int len = strlen(mounted_devices[i]->mountpoint);
    int exct = (path[len] == 0 || path[len] == '/');
    exct = (exct || (strcmp(mounted_devices[i]->mountpoint, "/") == 0));

    if (strncmp(mounted_devices[i]->mountpoint, path, len) == 0 && exct) {
      if (len > lastlen) {
        lastlen = len;
        lastdev = mounted_devices[i]->dev;
      }
    }
  }

  if (lastlen > 0) {
    *dev = lastdev;
    return SUCCESS;
  }

  return ENODEV;
}

static int vfs_dev_by_path(const char *path, blk_dev_t **dev) {
  if (dev == NULL)
    return EINVAL;

  for (int i = 0; i < _all_devices_pos; ++i) {
    if (_all_devices[i] == NULL)
      continue;

    if (strcmp(_all_devices[i]->devpath, path) != 0)
      continue;

    *dev = _all_devices[i]->dev;
    return SUCCESS;
  }

  return ENODEV;
}

static const char *vfs_driver_by_mountpoint(const char *path) {
  if (path == NULL)
    return NULL;

  const char *lastdrv;
  int lastlen = 0;

  for (int i = 0; i < mounted_devices_pos; ++i) {
    if (mounted_devices[i] == NULL)
      continue;

    /* Now not only exact mountpoint */
    int len = strlen(mounted_devices[i]->mountpoint);
    int exct = (path[len] == 0 || path[len] == '/');
    exct = (exct || (strcmp(mounted_devices[i]->mountpoint, "/") == 0));
    if (strncmp(mounted_devices[i]->mountpoint, path, len) == 0 && exct) {
      if (len > lastlen) {
        lastlen = len;
        lastdrv = mounted_devices[i]->driver->name;
      }
    }
  }

  if (lastlen > 0) {
    return lastdrv;
  }

  return NULL;
}

int vfs_register_driver(fs_driver_t *driver) {
  if (_fs_drivers_pos >= FS_DRIVERS_MAX)
    return ENOMEM;
  _fs_drivers[_fs_drivers_pos++] = driver;
  return SUCCESS;
}

int vfs_register_device(blk_dev_t *device, const char *devname) {
  if (devname == NULL)
    return EINVAL;
  if (_all_devices_pos >= MAX_DEVICES)
    return ENOMEM;

  int len = strlen("/dev");
  if (len < 0)
    return EINVAL;

  vfs_dev_t *devt = malloc(sizeof(vfs_dev_t));
  if (devt == NULL)
    return ENOMEM;

  devt->devpath = malloc(strlen(devname) + strlen("/dev/") + 1);
  if (devt->devpath == NULL) {
    free(devt);
    return ENOMEM;
  }

  memcpy(devt->devpath, "/dev/", 5);
  strcpy(devt->devpath + len + 1, devname);
  devt->dev = device;

  _all_devices[_all_devices_pos++] = devt;
  return SUCCESS;
}

void *vfs_opendir(const char *path) {
  if (path == NULL)
    return NULL;

  const char *driver = vfs_driver_by_mountpoint(path);
  if (driver == NULL)
    return NULL;

  blk_dev_t *dev;
  if (vfs_dev_by_mountpoint(path, &dev) != SUCCESS)
    return NULL;

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

int vfs_makefs(const char *path, const char *driver) {
  if (path == NULL)
    return EINVAL;

  fs_driver_t *drv;
  int err = vfs_ld_driver(driver, &drv);
  if (err != SUCCESS)
    return err;

  blk_dev_t *dev;
  if (vfs_dev_by_path(path, &dev) != SUCCESS)
    return ENODEV;

  err = drv->makefs(dev);

  return err;
}

int vfs_mountfs(const char *path, const char *mountpoint, const char *driver) {
  if (path == NULL || mountpoint == NULL)
    return EINVAL;

  fs_driver_t *drv;
  int err = vfs_ld_driver(driver, &drv);
  if (err != SUCCESS)
    return err;

  blk_dev_t *dev;
  if (vfs_dev_by_path(path, &dev) != SUCCESS)
    return ENODEV;

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
    for (int i = 0; i < _all_devices_pos; ++i) {
      vfs_dev_t *dev = _all_devices[i];
      if (dev == NULL || dev->devpath == NULL)
        continue;

      if (strcmp(dev->devpath, path) != 0)
        continue;

      /* Unmount device */
      for (int j = 0; j < mounted_devices_pos; ++j) {
        int exct = (mountpoint == NULL);

        if (!exct)
          exct = (strcmp(mountpoint, mounted_devices[j]->mountpoint) == 0);

        if (!exct)
          continue;

        if (mounted_devices[j]->dev->id != dev->dev->id)
          continue;

        mounted_devices[j]->driver->umountfs(dev->dev);
        free(mounted_devices[j]->mountpoint);
        free(mounted_devices[j]);

        // Move last
        --mounted_devices_pos;
        mounted_devices[j] = mounted_devices[mounted_devices_pos];
        --j;
      }
      return SUCCESS;
    }

    return ENOENT;
  }

  return EINVAL;
}

int vfs_creat(const char *path) { return -2; }

int vfs_mkdir(const char *path) { return -2; }

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
