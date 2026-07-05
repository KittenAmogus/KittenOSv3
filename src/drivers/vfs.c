#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <drivers/vfs.h>

vfs_driver_t *vfs_drivers[VFS_MAX_DRIVERS];
vfs_mount_t *vfs_mounted[VFS_MAX_MOUNTED];
vfs_device_t *vfs_devices[VFS_MAX_DEVICES];

int vfs_drivers_pos = 0;
int vfs_mounted_pos = 0;
int vfs_devices_pos = 0;

/* === Functions === */
/* Utility */
static int vfs_ld_driver(const char *driver_name, vfs_driver_t **driver) {
  if (driver == NULL || driver_name == NULL)
    return EINVAL;

  vfs_driver_t *drv;
  for (int i = 0; i < vfs_drivers_pos; ++i) {
    drv = vfs_drivers[i];
    if (drv == NULL)
      continue;

    if (strcmp(drv->driver->name, driver_name) == 0) {
      *driver = drv;
      return SUCCESS;
    }
  }

  return ENOENT;
}

static int vfs_ld_device(const char *devpath, vfs_device_t **device) {
  if (device == NULL || devpath == NULL)
    return EINVAL;

  vfs_device_t *dev;
  for (int i = 0; i < vfs_devices_pos; ++i) {
    dev = vfs_devices[i];
    if (dev == NULL)
      continue;

    if (strcmp(dev->devpath, devpath) == 0) {
      *device = dev;
      return SUCCESS;
    }
  }

  return ENOENT;
}

static const char *vfs_ld_mount(const char *fullpath, int *mount_id) {
  if (mount_id == NULL || fullpath == NULL)
    return NULL;

  size_t max_coll_len = 0;
  *mount_id = -1;

  vfs_mount_t *mnt;
  for (int i = 0; i < vfs_mounted_pos; ++i) {
    mnt = vfs_mounted[i];
    if (mnt == NULL)
      continue;

    size_t len = strlen(mnt->mountpoint);
    if (strncmp(mnt->mountpoint, fullpath, len) == 0 &&
        (fullpath[len] == 0 || fullpath[len] == '/')) {
      if (len > max_coll_len) {
        max_coll_len = len;
        *mount_id = i;
      }
    }
  }

  if (*mount_id == -1 || max_coll_len == 0)
    return NULL;

  // Root protection
  return fullpath + (max_coll_len > 1 ? max_coll_len : 0);
}

static const char *vfs_splitpath(const char *path) {
  if (path == NULL)
    return NULL;

  char *pathc = path;
  while (*pathc != 0)
    ++pathc;
  --pathc;

  while (*pathc == '/')
    --pathc;

  while (pathc > path) {
    if (*pathc == '/') {
      return pathc + 1;
    }

    --pathc;
  }

  return path;
}

/* Register */
int vfs_register_driver(fs_driver_t *driver) {
  if (driver == NULL)
    return EINVAL;
  if (vfs_drivers_pos >= VFS_MAX_DRIVERS)
    return ENOMEM;

  vfs_driver_t *vdriver = malloc(sizeof(vfs_driver_t));
  if (vdriver == NULL)
    return ENOMEM;

  vdriver->handle = NULL;
  vdriver->driver = driver;

  vfs_drivers[vfs_drivers_pos++] = vdriver;
  return SUCCESS;
}

int vfs_register_device(blk_dev_t *dev, char **devname) {
  if (devname == NULL || dev == NULL)
    return EINVAL;

  if (*devname == NULL) {
    *devname = malloc(7 + 1);
    if (*devname == NULL)
      return ENOMEM;

    strcpy(*devname, "unknown");
  }

  if (vfs_devices_pos >= VFS_MAX_DEVICES) {
    free(*devname);
    return ENOMEM;
  }

  const char *prefix = (const char *)DEVICE_PREFIX;
  char *devpath = malloc(strlen(*devname) + strlen(DEVICE_PREFIX) + 1);
  if (devpath == NULL) {
    free(*devname);
    return ENOMEM;
  }

  vfs_device_t *vdev = malloc(sizeof(vfs_device_t));
  if (vdev == NULL) {
    free(*devname);
    free(devpath);
    return ENOMEM;
  }

  strcpy(devpath, prefix);
  strcpy(devpath + strlen(prefix), *devname);

  vdev->devpath = malloc(strlen(devpath) + 1);
  if (vdev->devpath == NULL) {
    free(*devname);
    free(devpath);
    free(vdev);
    return ENOMEM;
  }

  vdev->dev = dev;
  strcpy(vdev->devpath, devpath);

  free(*devname);
  *devname = devpath;

  vfs_devices[vfs_devices_pos++] = vdev;
  return SUCCESS;
}

int vfs_unregister_driver(const char *driver_name) {
  if (driver_name == NULL)
    return EINVAL;

  vfs_driver_t *drv;
  for (int i = 0; i < vfs_drivers_pos; ++i) {
    drv = vfs_drivers[i];
    if (drv != NULL) {
      if (strcmp(drv->driver->name, driver_name) == 0) {
        free(drv);

        // Move last driver to freed pos
        --vfs_drivers_pos;
        vfs_drivers[i] = vfs_drivers[vfs_drivers_pos];

        return SUCCESS;
      }
    }
  }

  return ENOENT;
}

/* Device functions */
int vfs_mount_device(const char *device_path, const char *mountpoint,
                     const char *driver_name) {
  if (device_path == NULL || driver_name == NULL || mountpoint == NULL)
    return EINVAL;

  if (vfs_mounted_pos >= VFS_MAX_MOUNTED)
    return ENOMEM;

  vfs_driver_t *driver;
  if (vfs_ld_driver(driver_name, &driver) != SUCCESS)
    return ENOENT;

  vfs_device_t *device;
  if (vfs_ld_device(device_path, &device) != SUCCESS)
    return ENOENT;

  vfs_mount_t *mnt = malloc(sizeof(vfs_mount_t));
  if (mnt == NULL)
    return ENOMEM;

  mnt->mountpoint = malloc(strlen(mountpoint) + 1);
  if (mnt->mountpoint == NULL) {
    free(mnt);
    return ENOMEM;
  }

  strcpy(mnt->mountpoint, mountpoint);
  mnt->driver = driver->driver;
  mnt->dev = device->dev;
  mnt->fs_instance = NULL;

  int err = mnt->driver->mountfs(mnt->dev) != 0;
  if (err != SUCCESS) {
    free(mnt->mountpoint);
    free(mnt);
    return err;
  }

  vfs_mounted[vfs_mounted_pos++] = mnt;
  return SUCCESS;
}

int vfs_makefs_device(const char *device_path, const char *driver_name) {
  if (device_path == NULL || driver_name == NULL)
    return EINVAL;

  vfs_driver_t *drv;
  if (vfs_ld_driver(driver_name, &drv) != SUCCESS)
    return ENOENT;

  vfs_device_t *dev;
  if (vfs_ld_device(device_path, &dev) != SUCCESS)
    return ENOENT;

  return drv->driver->makefs(dev->dev);
}

int vfs_umount_device(const char *path) {
  if (path == NULL)
    return EINVAL;

  vfs_mount_t *mnt;
  int mnt_id;

  if (vfs_ld_mount(path, &mnt_id) == NULL)
    return ENOENT;

  mnt = vfs_mounted[mnt_id];
  mnt->driver->umountfs(mnt->dev);
  free(mnt->mountpoint);
  free(mnt);

  --vfs_mounted_pos;
  vfs_mounted[mnt_id] = vfs_mounted[vfs_mounted_pos];
  return SUCCESS;
}

/* File */
int vfs_creat(const char *path) {
  if (path == NULL)
    return EINVAL;

  int mount_id;
  const char *cleanpath = vfs_ld_mount(path, &mount_id);
  const char *filename = vfs_splitpath(path);
  int dirpath_len = (int)(filename - cleanpath);

  if (cleanpath == NULL)
    return ENOENT;

  vfs_mount_t *mnt = vfs_mounted[mount_id];
  return mnt->driver->creat(mnt->dev, cleanpath, dirpath_len);
}

int vfs_mkdir(const char *path) {
  if (path == NULL)
    return EINVAL;

  int mount_id;
  const char *cleanpath = vfs_ld_mount(path, &mount_id);
  const char *filename = vfs_splitpath(path);
  int dirpath_len = (int)(filename - cleanpath);

  if (cleanpath == NULL)
    return ENOENT;

  vfs_mount_t *mnt = vfs_mounted[mount_id];
  return mnt->driver->mkdir(mnt->dev, cleanpath, dirpath_len);
}

int vfs_rmdir(const char *path) {
  if (path == NULL)
    return EINVAL;

  int mount_id;
  const char *cleanpath = vfs_ld_mount(path, &mount_id);
  const char *filename = vfs_splitpath(path);
  int dirpath_len = (int)(filename - cleanpath);

  if (cleanpath == NULL)
    return ENOENT;

  vfs_mount_t *mnt = vfs_mounted[mount_id];
  return mnt->driver->rmdir(mnt->dev, cleanpath, dirpath_len);
}

int vfs_unlink(const char *path) {
  if (path == NULL)
    return EINVAL;

  int mount_id;
  const char *cleanpath = vfs_ld_mount(path, &mount_id);
  const char *filename = vfs_splitpath(path);
  int dirpath_len = (int)(filename - cleanpath);

  if (cleanpath == NULL)
    return ENOENT;

  vfs_mount_t *mnt = vfs_mounted[mount_id];
  return mnt->driver->unlink(mnt->dev, cleanpath, dirpath_len);
}
