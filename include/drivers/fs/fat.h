#ifndef _FS_FAT_H
#define _FS_FAT_H

#include <drivers/disk.h>

#define FAT_NAME "fat"

typedef struct fat_bpb {
  struct {
    char JmpBoot[3]; // x86 JMP instruction to boot OS
    char OEMName[8]; // Name of system created volume
  } __attribute__((packed)) BS;

  uint16_t BytsPerSec; // Bytes in Sector  (512)
  uint8_t SecPerClus;  // Sectors in Cluster
  uint16_t RsvdSecCnt; // Sectors in Reserved area (32)
  uint8_t NumFATs;     // FAT Copies number  (2)
  uint16_t RootEntCnt; // Entries in root (0)
  uint16_t TotSec16;   // 0, value is in 32 // All sectors in volume
  uint8_t Media;       // 0xF0 - Floppy, FAT[0] = (Media & 0xFF)
  uint16_t FATSz16;    // 0, value is in 32 // sectors occupied by FAT
  uint16_t SecPerTrk;  // Sectors in Track (only for Geometry disks)
  uint16_t NumHeads;   // Heads on disk (only for Geometry disks)
  uint32_t HiddSec;    // Sectors before FAT(offset) (0 for floppies and
                       // non-partitioned)
  uint32_t TotSec32;   // Sectors on disk (32bit)

  union {
    struct {
    } __attribute__((packed)) fat16;

    struct {
      uint32_t FATSz32; // Sectors occupied by FAT

      union {
        struct {
          uint16_t activeFAT : 4;       // Active FAT (0-3)
          uint16_t rsvd0 : 3;           // * Starting from 0
          uint16_t activeFlag : 1;      // Active flag (7)
          uint16_t rsvd1 : 8;           // * 0 - all active & mirrored
        } __attribute__((packed)) pack; // * 1 - only one FAT active
        uint16_t raw;                   // Raw ExtFlags
      } __attribute__((packed)) ExtFlags;

      uint16_t FSVer;     // FAT32 version
      uint32_t RootClus;  // First cluster of Root (2)
      uint16_t FSInfo;    // Sector of FSInfo(offset from top) (1)
      uint16_t BkBootSec; // Backup boot sector(offset from top) (6)
      char rsvd0[12];     // Reserved (all zeroes)

      struct {
        uint8_t DrvNum;       // Drive number 0x00 floppy, 0x80 fixed
        uint8_t Reserved;     // Reserved in DOS/Win9, used in WinNT (0)
        uint8_t BootSig;      // 0x29 (Extended boot signature)
        uint32_t VolId;       // Volume serial number
        char VolLab[11];      // Volume label, "NO NAME " when not set
        char FilSysType[8];   // "FAT32   "
        char BootCode32[420]; // Bootstrap program
        uint16_t Sign;        // 0xAA55 (Valid boot sector)
      } __attribute__((packed)) BS;

    } __attribute__((packed)) fat32;
  } __attribute__((packed)) data;

} __attribute__((packed)) fat_bpb_t;

typedef struct fat_fsinfo {
  uint32_t LeadSig;   // 0x41615252
  char rsvd0[480];    // Reserved (0)
  uint32_t StructSig; // 0x61417272
  uint32_t FreeCount; // free cluster count on volume (0xFFFFFFFF - unknown)
  uint32_t NxtFree;   // Last allocated cluster (0xFFFFFFFF - search from 2)
  char rsvd1[12];     // Reserved (0)
  uint32_t TrailSig;  // 0xAA550000
} __attribute__((packed)) fat_fsinfo_t;

typedef enum {
  FAT_ATTR_READ_ONLY = 0x01,
  FAT_ATTR_HIDDEN = 0x02,
  FAT_ATTR_SYSTEM = 0x04,
  FAT_ATTR_VOLUME_ID = 0x08,
  FAT_ATTR_DIRECTORY = 0x10,
  FAT_ATTR_ARCHIVE = 0x20,
  FAT_ATTR_LFN = 0x0F
} fat_attribute_e;

typedef struct fat_dirent {
  char Name[11];        // Filename
  uint8_t Attr;         // File attributes
  uint8_t NTRes;        // Optional flags
  uint8_t CrtTimeTenth; // Subseconds (0)
  uint16_t CrtTime;     // Creation time
  uint16_t CrtDate;     // Creation date
  uint16_t LstAccDate;  // Last access date (read/write)
  uint16_t FstClusHI;   // Upper part of first cluster number
  uint16_t WrtTime;     // Last change time (write time)
  uint16_t WrtDate;     // Last change date (write date)
  uint16_t FstClusLO;   // Lower part of first cluster number
  uint32_t FileSize;    // Size in bytes (DIR = 0)
} __attribute__((packed)) fat_dirent_t;

typedef struct fat_lfnent {
  uint8_t Ord;        // Sequence number (0x40 = end)
  uint16_t name1[5];  // 0-4 chars
  uint8_t Attr;       // FAT_ATTR_LFN
  uint8_t Type;       // 0
  uint8_t Chksum;     // Checksum of the SFN with this entry
  uint16_t name2[6];  // 5-10 chars
  uint16_t FstClusLO; // 0
  uint16_t name3[2];  // 11-12 chars
} __attribute__((packed)) fat_lfnent_t;

typedef struct {
  uint32_t BytsPerSec;   // Sector size
  uint32_t BytsPerClus;  // Cluster size
  uint32_t FatStartSec;  // First fat sector
  uint32_t DataStartSec; // First data sector
  uint32_t NxtFree;      // Last writed cluster
  uint32_t RootClus;     // Root cluster
  uint32_t FreeCount;    // Free clusters
} fat_instance_t;

typedef struct {
  blk_dev_t *dev;             // Device
  uint32_t current_cluster;   // Last read cluster
  uint32_t current_sector;    // Last read sector
  uint32_t start_sector;      // First sector in cluster
  uint32_t byte_offset;       // Last read byte
  uint8_t buffer[BLOCK_SIZE]; // Buffer for sector
} fat_dir_handle_t;

extern fs_driver_t fat_driver;

#endif // _FS_FAT_H
