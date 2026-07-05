#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <drivers/fs/fat.h>
#include <drivers/vfs.h>

/* Print debug info */
#define _DEBUG
#ifdef _DEBUG
#include <stdio.h>
#else
#define puts
#define printf
#define putchar
#endif

#define DEV_FROM_HANDLE(handle)                                                \
  ((blk_dev_t *)(((fat_dir_handle_t *)handle)->dev))
#define INST_FROM_HANDLE(handle)                                               \
  ((fat_instance_t *)(DEV_FROM_HANDLE(handle)->fs_private))
#define READ_SECTOR(dev, sector, buffer, count)                                \
  (dev->read_sector(dev, sector, buffer, count))
#define WRITE_SECTOR(dev, sector, buffer, count)                               \
  (dev->write_sector(dev, sector, buffer, count))

/* Driver name */
const char fat_name[] = "fat";

/* === Utility functions === */
static int fat_find_free_cluster(fat_dir_handle_t *handle);
static int fat_next_cluster(fat_dir_handle_t *handle);
static int fat_alloc_cluster(fat_dir_handle_t *handle);
static int fat_next_dirent(fat_dir_handle_t *handle);
static int fat_load_dirent(fat_dir_handle_t *handle, fat_dirent_t *dirent);
static int fat_store_dirent(fat_dir_handle_t *handle, fat_dirent_t *dirent);
static int *fat_shorten_name(const char *long_name, char *dest, int colls);
static int fat_find_free_dirent(fat_dir_handle_t *handle, int skip_deleted);

/* === Driver functions === */
/* Device driver */
static int fat_makefs(blk_dev_t *dev);
static int fat_mountfs(blk_dev_t *dev);
static int fat_umountfs(blk_dev_t *dev);

/* Directory drivers */
static void *fat_opendir(blk_dev_t *dev, const char *path);
static int fat_readdir(void *handle, dirent_t *dirent);
static void fat_closedir(void *handle);

/* File driver */
static int fat_creat(blk_dev_t *dev, const char *path, const char *filename);
static int fat_mkdir(blk_dev_t *dev, const char *path, const char *filename);
static int fat_rmdir(blk_dev_t *dev, const char *path, const char *filename);
static int fat_unlink(blk_dev_t *dev, const char *path, const char *filename);

/* Create driver */
fs_driver_t fat_driver = {.name = fat_name,
                          .opendir = fat_opendir,
                          .readdir = fat_readdir,
                          .closedir = fat_closedir,
                          .makefs = fat_makefs,
                          .mountfs = fat_mountfs,
                          .umountfs = fat_umountfs,
                          .creat = fat_creat,
                          .mkdir = fat_mkdir,
                          .rmdir = fat_rmdir,
                          .unlink = fat_unlink};
