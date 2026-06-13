#include "ata.h"

#include "drivers/io/io.h"

static void ata_wait_ready(void) {
  while (inb(ATA_REG_COMMAND) & ATA_STATUS_BSY);    // Wait while busy
  while (!(inb(ATA_REG_COMMAND) & ATA_STATUS_DRQ)); // Wait until ready
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
}

