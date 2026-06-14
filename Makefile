# Tools
ASM = nasm
CC  = gcc
LD  = ld

# Flags
SFLAGS  = -f elf32
CFLAGS  = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -Wno-unused-parameter
LDFLAGS = -melf_i386

# Directories
BUILD = build
SRC_INNER = $(shell find src -mindepth 1 -type d)
BLD_INNER = $(patsubst src/%,build/%,$(SRC_INNER))

# Custom includes
LIBS_DIR  = src/libs
LIBS = $(shell find $(LIBS_DIR) -mindepth 1 -type d)
LIBS_INC = $(patsubst %,-I%,$(LIBS))

# Sources
SSOURCES = $(shell find src -name '*.s')
CSOURCES = $(shell find src -name '*.c')

# Objects
SOBJECTS = $(patsubst src/%.s,$(BUILD)/%.s.o,$(SSOURCES))
COBJECTS = $(patsubst src/%.c,$(BUILD)/%.c.o,$(CSOURCES))
OBJECTS = $(SOBJECTS) $(COBJECTS)

# Link config
LDFILE = src/link.ld

# Target
TARGET = kernel.elf

# ISO
ISODIR    = iso
ISOFILE   = os.iso
GRUBFILES = grub/grub.cfg

DISKIMG = ./hda.img

# BOCHS
BOCHS = bochs
BFLAGS = -f ./bochsrc.txt -q

# QEMU
QEMU = qemu-system-i386
QFLAGS = -cdrom $(ISOFILE) -hda $(DISKIMG) -boot d


.PHONY: all clean run

all: $(TARGET) iso

$(BUILD)/%.s.o: src/%.s | $(BUILD)
	@echo "-- Assembling $< -> $@"
	$(ASM) $(SFLAGS) $< -o $@

$(BUILD)/%.c.o: src/%.c | $(BUILD)
	@echo "-- Compiling $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS_INC) -Isrc

$(TARGET): $(OBJECTS)
	@echo "-- Linking $@"
	$(LD) $(LDFLAGS) -T $(LDFILE) -o $(TARGET) $(OBJECTS) $(LIBS_INC) -Isrc

clean:
	@echo "-- Cleaning up"
	@rm -rf $(BLD_INNER) $(BUILD) $(ISODIR) $(TARGET)

$(BUILD):
	@echo "-- Creating directories"
	@mkdir -p $(BUILD) $(BLD_INNER)

iso: $(TARGET)
	@echo "-- Creating ISO structure"
	@mkdir -p $(ISODIR)/boot/grub
	@cp $(TARGET) $(ISODIR)/boot/
	@cp $(GRUBFILES) $(ISODIR)/boot/grub/
	@echo "-- Burning ISO file"
	@grub-mkrescue -o $(ISOFILE) $(ISODIR)

bochs:
	$(BOCHS) $(BFLAGS)

qemu:
	$(QEMU) $(QFLAGS)

hda:
	@dd if=/dev/urandom of=$(DISKIMG) bs=1M count=32

