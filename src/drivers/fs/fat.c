#include <drivers/disk.h>
#include <drivers/fs/fat.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

static void fat_mkfs_fillConstants(fat_bpb_t *bpb) {
  /* Set constants */
  bpb->BytsPerSec = 512;
  bpb->NumFATs = 2;
  bpb->RootEntCnt = 0;
  bpb->TotSec16 = 0;
  bpb->FATSz16 = 0;
  bpb->Media = 0xF8;              // Fixed media
  bpb->data.fat32.FSVer = 0x0000; // 0.0
  bpb->data.fat32.RootClus = 2;
  bpb->data.fat32.FSInfo = 1;
  bpb->data.fat32.BkBootSec = 6;
  bpb->data.fat32.BS.BootSig = 0x29;
  bpb->data.fat32.BS.Sign = 0xAA55;

  /* Fill constants */
  memcpy((void *)&(bpb->BS.JmpBoot[0]), (uint8_t[]){0xEB, 0x58, 0x90}, 3);
  memcpy((void *)&(bpb->BS.OEMName[0]), "KITTENOS", 8);
  memcpy((void *)&(bpb->data.fat32.BS.FilSysType[0]), (const void *)"FAT32   ",
         8);
  memcpy((void *)&(bpb->data.fat32.BS.VolLab[0]), (const void *)"KITTENOS V4",
         11);

  // TODO: Remove hardcode
  bpb->data.fat32.BS.DrvNum = 0x80;
  bpb->RsvdSecCnt = 32;
  bpb->SecPerClus = 2;
}

static void fat_mkfs_fillDynamic(fat_bpb_t *bpb, blk_dev_t *dev) {
  /* Fill from dev */
  bpb->TotSec32 = dev->total_sectors;
  bpb->HiddSec = dev->fs_offset;

  // TODO: Remove hardcode
  bpb->data.fat32.BS.VolId = 0xDEADC0DE;

  /* Fill dynamic */
  bpb->data.fat32.FATSz32 = bpb->TotSec32 - (bpb->RsvdSecCnt + bpb->HiddSec);
}

static void fat_mkfs_fillFsinfo(fat_fsinfo_t *fsi, int free_cnt) {
  fsi->LeadSig = 0x41615252;
  fsi->StructSig = 0x61417272;
  fsi->TrailSig = 0xAA550000;

  // TODO: Remove hardcode
  fsi->FreeCount = free_cnt;
  fsi->NxtFree = 3; // Root(2) + 1
}

static int fat_makefs(blk_dev_t *dev) {
  if (dev == NULL)
    return EINVAL;

  void *buffer = malloc(BLOCK_SIZE);
  if (buffer == NULL)
    return ENOMEM;

  /* Fill BPB */
  fat_bpb_t *bpb = (fat_bpb_t *)buffer;
  memset(buffer, 0, BLOCK_SIZE);

  fat_mkfs_fillConstants(bpb);
  fat_mkfs_fillDynamic(bpb, dev);
  /* Write buffer */
  dev->write_sector(dev, 0, buffer, 1);
  dev->write_sector(dev, 6, buffer, 1); // Backup
  // TODO: Add MBR support

  /* Fill FSI */
  int total_size = (bpb->TotSec32 - bpb->RsvdSecCnt - bpb->HiddSec) -
                   (bpb->data.fat32.FATSz32 << 1);
  fat_fsinfo_t *fsi = (fat_fsinfo_t *)buffer;
  memset(buffer, 0, BLOCK_SIZE);

  fat_mkfs_fillFsinfo(fsi, total_size);
  /* Write buffer */
  dev->write_sector(dev, 1, buffer, 1);
  dev->write_sector(dev, 7, buffer, 1); // Backup
  // TODO: Add MBR support

  /* Fill reserved blocks */
  memset(buffer, 0, BLOCK_SIZE);
  for (int i = 2; i < 6; ++i) {
    dev->write_sector(dev, i, buffer, 1);
    // TODO: Add MBR support
  }

  free(buffer);
  return SUCCESS;
}

static int fat_mountfs(blk_dev_t *dev) {
  if (dev == NULL)
    return EINVAL;

  if (dev->fs_private != NULL)
    return EEXIST;

  void *buffer = malloc(BLOCK_SIZE);
  if (buffer == NULL)
    return ENOMEM;

  int err = dev->read_sector(dev, dev->fs_offset + 0, buffer, 1);
  if (err != SUCCESS) {
    free(buffer);
    return err;
  }

  fat_instance_t *inst = malloc(sizeof(fat_instance_t));
  fat_bpb_t *bpb = (fat_bpb_t *)buffer;

  if (inst == NULL) {
    free(buffer);
    return ENOMEM;
  }

  if (bpb->data.fat32.BS.Sign != 0xAA55) {
    free(inst);
    free(buffer);
    return EINVAL; // Invalid signature
  }

  // Fill bpb data
  inst->BytsPerSec = bpb->BytsPerSec;
  inst->RootClus = bpb->data.fat32.RootClus;
  inst->BytsPerClus = (inst->BytsPerSec * bpb->SecPerClus);
  inst->FatStartSec = dev->fs_offset + bpb->RsvdSecCnt;
  inst->DataStartSec =
      inst->FatStartSec + (bpb->NumFATs * bpb->data.fat32.FATSz32);

  err = dev->read_sector(dev, dev->fs_offset + 1, buffer, 1);
  if (err != SUCCESS) {
    free(inst);
    free(buffer);
    return err;
  }

  // Fill fsi data
  fat_fsinfo_t *fsi = (fat_fsinfo_t *)buffer;

  if (fsi->LeadSig != 0x41615252 || fsi->StructSig != 0x61417272 ||
      fsi->TrailSig != 0xAA550000) {
    free(inst);
    free(buffer);
    return EINVAL; // Invalid signature
  }

  inst->FreeCount = fsi->FreeCount;
  inst->NxtFree = fsi->NxtFree;

  // Set instance
  dev->fs_private = inst;

  free(buffer);
  return SUCCESS;
}

static int fat_umountfs(blk_dev_t *dev) {
  free(dev->fs_private);
  dev->fs_private = NULL;

  return SUCCESS;
}

static void *fat_opendir(const char *path) {}
static void fat_closedir(void *handle) {}
static int fat_readdir(void *handle, dirent_t *dirent) {}

fs_driver_t fat_driver = {.name = FAT_NAME,
                          .opendir = fat_opendir,
                          .readdir = fat_readdir,
                          .closedir = fat_closedir,
                          .makefs = fat_makefs,
                          .mountfs = fat_mountfs,
                          .umountfs = fat_umountfs};
