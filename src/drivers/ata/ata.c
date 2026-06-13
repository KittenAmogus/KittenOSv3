#include "ata.h"

#include "drivers/io/io.h"

static void ata_wait_ready(void) {
  while (inb(ATA_REG_COMMAND) & ATA_STATUS_BSY);    // Wait while busy
  // while (!(inb(ATA_REG_COMMAND) & ATA_STATUS_DRQ)); // Wait until ready
}

uint32_t ata_identify_disk(uint16_t *buff) {
  // Disk
  outb(ATA_REG_DRIVE, 0xA0);

  // Zeroes
  outb(ATA_REG_SECCOUNT, 0);
  outb(ATA_REG_LBA_LO, 0);
  outb(ATA_REG_LBA_MID, 0);
  outb(ATA_REG_LBA_HI, 0);
 
  // Command
  outb(ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

  if (inb(ATA_REG_COMMAND) == 0) return 1;  // No disk

  // Read data
  ata_wait_ready();
  insw(ATA_REG_DATA, buff, 256);
  return 0;
}

void ata_read_sector(uint32_t lba, uint16_t *buff) {
  uint8_t drive_byte = 0xE0 | ((lba >> 24) & 0x0F); // LBA + ADDR
  outb(ATA_REG_DRIVE, drive_byte);

  // Read count
  outb(ATA_REG_SECCOUNT, 1);  // One sector

  // Addr
  outb(ATA_REG_LBA_LO,  lba & 0xFF);
  outb(ATA_REG_LBA_MID, (lba >> 8 ) & 0xFF);
  outb(ATA_REG_LBA_HI,  (lba >> 16) & 0xFF);

  // Read cmd
  outb(ATA_REG_COMMAND, ATA_CMD_READ_PIO);

  ata_wait_ready();               // Wait for disk to prepare
  insw(ATA_REG_DATA, buff, 256);  // Read 256 words
}

void ata_write_sector(uint32_t lba, const uint16_t *buff) {
  uint8_t drive_byte = 0xE0 | ((lba >> 24) & 0x0F); // LBA + ADDR
  outb(ATA_REG_DRIVE, drive_byte);

  // Write count
  outb(ATA_REG_SECCOUNT, 1);  // One sector

  // Addr
  outb(ATA_REG_LBA_LO, lba & 0xFF);
  outb(ATA_REG_LBA_MID, (lba >> 8 ) & 0xFF);
  outb(ATA_REG_LBA_HI,  (lba >> 16) & 0xFF);

  // Write cmd
  outb(ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

  ata_wait_ready();               // Wait for disk to prepare
  outsw(ATA_REG_DATA, buff, 256); // Write 256 words
  while (inb(ATA_REG_COMMAND) & ATA_STATUS_BSY);  // Wait for write
}

