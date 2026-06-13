#include "apps.h"

#include "stdio.h"
#include "errno.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "random.h"

#include "kernel/fs/fs.h"
#include "drivers/ata/ata.h"

extern superblock_t *superblock_mounted;

static const char *noargs_echo = "Usage: echo <string>";

static uint32_t app_clear(char *args) {
  clear_screen();
  return 0;
}

static uint32_t app_echo(char *args) {
  if (args == NULL) {
    puts(noargs_echo);
    return 1;
  }
  puts(args);
  return 0;
}

static uint32_t app_help(char *args) {
  puts("Available commands: ");
  for (uint32_t i=0; i<app_count; ++i) {
    app_t *app = &(app_table[i]);
    if (app == NULL || app->name == NULL || app->descr == NULL)
      printf(" * [CORRUPTED]\n");
    else
      printf(" * %s - %s\n", app->name, app->descr);
  }

  return 0;
}

static uint32_t app_fetch(char *args) {
  puts(" /\\_/\\  OS name | KittenOSv3");
  puts("( o.o ) OS type | 32-bit Monolithic");
  puts("  >^<   Author  | @kittenamogus\n");
  puts("        <><><><><><><><><><><><><><>");
  printf("        RAM     | %d/%d MB free\n",
    get_heap_free() >> 20, get_heap_size() >> 20);
  return 0;
}

static uint32_t app_malloc(char *args) {
  size_t size;

  if (args[0] == '0' && args[1] == 'x') {
    size = atoh(args + 2);
  } else {
    size = atoi(args);
  }

  if ((int32_t)size <= 0) {
    puts("Cannot allocate <= 0 bytes");
    return 2;
  }

  uint8_t *buff = malloc(size);
  if (buff == NULL) {
    puts("Alloc error, not enough RAM");
    return 1;
  }
  printf(
    "Allocated %d (0x%x) bytes, addr: %d (0x%x)\n",
    size, size, (uint32_t)buff, (uint32_t)buff);
  return 0;
}


static uint32_t app_free(char *args) {
  void *ptr;

  if (args[0] == '0' && args[1] == 'x') {
    ptr = (void*)atoh(args + 2);
  } else {
    ptr = (void*)atoi(args);
  }

  if (ptr == NULL || ptr <= META_SIZE) {
    puts("Invalid pointer, too small");
    return 2;
  }

  Block *b = (Block*)((uint8_t*)ptr - META_SIZE);
  if (b->magic != BLOCK_MAGIC) {
    if (b->magic == ~BLOCK_MAGIC) {
      puts("Double-free attempt!");
      return 3;
    }
    puts("Invalid addr");
    return 1;
  }

  free(ptr);
  puts("Freed block");
  return 0;
}

static uint32_t app_ls(char *args) {
  if (superblock_mounted == NULL) {
    puts("No mounted disk");
    return ENODEV;
  }

  // Allocate buffer
  void *buffer = malloc(BLOCK_SIZE);
  if (buffer == NULL) return ENOMEM;
  memset(buffer, 0, BLOCK_SIZE);

  // Allocate inode
  inode_t *inode = malloc(sizeof(inode_t));
  if (inode == NULL) {
    free(buffer);
    return ENOMEM;
  }
  memset(inode, 0, sizeof(inode_t));

  // Variables
  uint16_t *raw = (uint16_t*)buffer;
  uint8_t *rawdata;
  dirent_t *file;

  // Read
  ata_read_sector(INODE_TABLE_START, raw);
  memcpy(inode, buffer, sizeof(inode_t));

  if (inode->type != FS_FILE_DIR) {
    free(buffer);
    free(inode);
    puts("Root is not a directory");
    return ENOTDIR;
  }
  puts("/");  // Root

  // Check all data segments  TODO: extendable segment
  for (uint32_t i=0; i<DATA_CNT; ++i) {
    if (inode->data_blocks[i] == 0) break;
    ata_read_sector(inode->data_blocks[i], raw);

    // Pointers
    file = (dirent_t*)raw;
    rawdata = (uint8_t*)raw;

    // File loop
    do {
      if (file->type == 0) break; // Undefined file(end)
      printf("|-%s (%d)\n", file->name, file->type);
 
      // Next file
      rawdata += file->rec_len;
      file = (dirent_t*)rawdata;

      // While rawdata is inside one block
    } while ((uint32_t)file < (uint32_t)((uint8_t*)buffer + BLOCK_SIZE));
  }

  // Free RAM
  free(inode);
  free(buffer);
  return SUCCESS;
}

static uint32_t app_mkfs(char *args) {
  uint32_t status = mkfs();
  if (status != 0)
    printf("Failed to make fs: %d\n", status);
  return status;
}

static uint32_t app_mnt(char *args) {
  uint32_t status = mount();
  if (status != 0)
    printf("Failed to mount: %d\n", status);
  return status;
}

static uint32_t app_touch(char *args) {
  if (args == NULL) return EINVAL;

  inode_t *src = malloc(sizeof(inode_t));
  if (src == NULL) return ENOMEM;
  memset(src, 0, sizeof(inode_t));

  puts("Creating file...");
  // DEBUG
  src->size = 1024;
  src->type = FS_FILE_FILE;
  src->data_blocks[0] = DATA_TABLE_START;
  uint32_t status = create_inode(src);
  if (status != 0)
    printf("Failed to create inode %d\n", status);

  return status;
}

app_t app_table[] = {
  {"clear",   "Fills screen with empty chars", app_clear},
  {"echo",    "Prints args in stdout", app_echo},
  {"help",    "Prints list of apps with their descriptions", app_help},
  {"fetch",   "Prints some OS and hardware info", app_fetch},
  {"malloc",  "Allocate <args> bytes", app_malloc},
  {"free",    "Free RAM block at <args> addr", app_free},
  {"ls",      "Prints all files in current dir (now only root)", app_ls},
  {"mount",   "Mounts disk (formated in KittenFS)", app_mnt},
  {"mkfs",    "Format disk in KittenFS", app_mkfs},
  {"touch",   "Creates inode (and link later)", app_touch},
};
const uint32_t app_count = sizeof(app_table) / sizeof(app_t);

