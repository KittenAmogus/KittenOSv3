#ifndef ATA_H
#define ATA_H

#include "stdint.h"

// Primary bus
#define ATA_REG_DATA       0x1F0  // Data
#define ATA_REG_FEATURES   0x1F1  // Parameters / errors
#define ATA_REG_SECCOUNT   0x1F2  // Sector count
#define ATA_REG_LBA_LO     0x1F3  // LBA 0-7
#define ATA_REG_LBA_MID    0x1F4  // LBA 8-15
#define ATA_REG_LBA_HI     0x1F5  // LBA 16-23
#define ATA_REG_DRIVE      0x1F6  // LBA 24-27
#define ATA_REG_COMMAND    0x1F7  // Command

// Disk commands
#define ATA_CMD_READ_PIO   0x20   // Read sectors
#define ATA_CMD_WRITE_PIO  0x30   // Write sectors

// Disk status
#define ATA_STATUS_ERR     0x01   // Error
#define ATA_STATUS_DRQ     0x08   // Data request
#define ATA_STATUS_SRV     0x10   // Service
#define ATA_STATUS_DF      0x20   // Drive fault
#define ATA_STATUS_RDY     0x40   // Ready
#define ATA_STATUS_BSY     0x80   // Busy

void ata_read_sector(uint32_t lba, uint16_t *buff);
void ata_write_sector(uint32_t lba, const uint16_t *buff);

#endif // ATA_H

