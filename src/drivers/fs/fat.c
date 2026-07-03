#include <drivers/disk.h>
#include <drivers/fs/fat.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

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

  /*
   *
    inst->BytsPerSec = bpb->BytsPerSec;
    inst->RootClus = bpb->data.fat32.RootClus;
    inst->BytsPerClus = (inst->BytsPerSec * bpb->SecPerClus);
    inst->FatStartSec = dev->fs_offset + bpb->RsvdSecCnt;
    inst->DataStartSec =
        inst->FatStartSec + (bpb->NumFATs * bpb->data.fat32.FATSz32);
   * */

  int DataStartSec = ((dev->fs_offset + bpb->RsvdSecCnt) +
                      (bpb->NumFATs * bpb->data.fat32.FATSz32));
  int root_sec =
      (DataStartSec + ((bpb->data.fat32.RootClus - 2) * bpb->SecPerClus));

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

  /* Create root */
  // dev->read_sector(dev, root_sec, buffer, 1);

  /* Fill root */
  fat_dirent_t *fat_dirent = (fat_dirent_t *)buffer + 0;
  memcpy(fat_dirent->Name, ".", 1);
  fat_dirent->Attr = FAT_ATTR_DIRECTORY;

  fat_dirent = (fat_dirent_t *)buffer + 1;
  memcpy(fat_dirent->Name, "..", 2);
  fat_dirent->Attr = FAT_ATTR_DIRECTORY;

  fat_dirent = (fat_dirent_t *)buffer + 2;
  memcpy(fat_dirent->Name, "TEST.TXT", 8);
  fat_dirent->Attr = 0x00;

  dev->write_sector(dev, root_sec, buffer, 1);

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

  if (bpb->data.fat32.BS.Sign != 0xAA55)
    goto cleanup;

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
      fsi->TrailSig != 0xAA550000)
    goto cleanup;

  inst->FreeCount = fsi->FreeCount;
  inst->NxtFree = fsi->NxtFree;

  // Set instance
  dev->fs_private = inst;

  free(buffer);
  return SUCCESS;

cleanup:
  free(inst);
  free(buffer);
  return EINVAL;
}

static int fat_umountfs(blk_dev_t *dev) {
  free(dev->fs_private);
  dev->fs_private = NULL;

  return SUCCESS;
}

static void fat_closedir(void *handle) {}

static int fat_readdir(void *handle, dirent_t *dirent) {
  if (handle == NULL || dirent == NULL)
    return -10;

  fat_dir_handle_t *fat_handle = (fat_dir_handle_t *)handle;
  fat_instance_t *inst = (fat_instance_t *)fat_handle->dev->fs_private;

  if (inst == NULL)
    return -11;

  // Read sector
  fat_handle->dev->read_sector(fat_handle->dev, fat_handle->current_sector,
                               fat_handle->buffer, 1);

  // Create dirent
  fat_dirent_t *fat_dirent =
      (fat_dirent_t *)((char *)fat_handle->buffer + fat_handle->byte_offset);

  /*
  uint32_t root_clus =
      (inst->DataStartSec +
       ((inst->RootClus - 2) * (inst->BytsPerClus / inst->BytsPerSec)));

  if ((uint8_t)fat_dirent->Name[0] == 0x00 &&
      (fat_handle->current_sector != root_clus ||
       fat_handle->byte_offset != 0 ||
       fat_handle->current_sector != inst->RootClus))
    return -12;
  */
  if ((uint8_t)fat_dirent->Name[0] == 0x00)
    return -1;

  if ((uint8_t)fat_dirent->Name[0] == 0xE5 || fat_dirent->Attr == 0x0F) {
    return -13;
  }

  // Fill dirent
  memcpy(dirent->name, fat_dirent->Name, 11);
  dirent->name[11] = 0;
  dirent->type = (fat_dirent->Attr & 0x10 ? DIRENT_DIR : DIRENT_FILE);

  // Next dirent
  fat_handle->byte_offset += sizeof(fat_dirent_t);

  // Next sector
  if (fat_handle->byte_offset >= BLOCK_SIZE) {
    fat_handle->byte_offset = 0;
    fat_handle->current_sector++;

    // Next cluster
    uint32_t SecPerClus = (inst->BytsPerClus / inst->BytsPerSec);
    if ((fat_handle->current_sector - fat_handle->start_sector) >= SecPerClus) {
      return -2;
    }
  }

  return 0;
}

static void *fat_opendir(blk_dev_t *dev, const char *path) {
  fat_dir_handle_t *handle = malloc(sizeof(fat_dir_handle_t));
  if (handle == NULL || path == NULL)
    return NULL;

  // Get mount
  fat_instance_t *inst = (fat_instance_t *)dev->fs_private;
  if (inst == NULL)
    goto cleanup;

  // Get root cluster
  int err = dev->read_sector(dev, inst->DataStartSec + inst->RootClus,
                             handle->buffer, 1);
  if (err != SUCCESS)
    goto cleanup;

  handle->current_cluster = inst->RootClus;
  handle->start_sector = inst->RootClus;
  handle->current_sector =
      inst->DataStartSec +
      ((handle->current_cluster - 2) * (inst->BytsPerClus / inst->BytsPerSec));
  handle->byte_offset = 0;
  handle->dev = dev;

  // Just root
  if (strcmp("/", path) == 0)
    return handle;

  char **path_parts = malloc(sizeof(path_parts) * 16);
  if (path_parts == NULL)
    goto cleanup;

  int pc = 0;
  char *p = (char *)path + 1; // Next from '/'
  do {
    path_parts[pc++] = p;
    p = vfs_next_subd(p);
  } while (p != NULL);

  // Find directory
  fat_dirent_t *dirent =
      malloc(sizeof(fat_dirent_t)); // (fat_dirent_t *)handle->buffer;
  if (dirent == NULL)
    goto cleanup;

  for (int i = 0; i < pc; ++i) {
    printf("Searching: '%s'...\n", path_parts[i]);
  }

  return handle;

cleanup:
  free(handle);
  return NULL;
}

fs_driver_t fat_driver = {.name = FAT_NAME,
                          .opendir = fat_opendir,
                          .readdir = fat_readdir,
                          .closedir = fat_closedir,
                          .makefs = fat_makefs,
                          .mountfs = fat_mountfs,
                          .umountfs = fat_umountfs};
