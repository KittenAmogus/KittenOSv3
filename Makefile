# Tools
ASM = nasm
CC  = gcc
LD  = ld

# Flags
SFLAGS  = -f elf32
CFLAGS  = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra
LDFLAGS = -melf_i386

# Directories
BUILD = build
SRC_INNER = $(shell find src -mindepth 1 -type d)
BLD_INNER = $(patsubst src/%,build/%,$(SRC_INNER))

# Custom includes
INCLUDE_INC = -isystem ./src/include -I./src

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
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDE_INC)

$(TARGET): $(OBJECTS)
	@echo "-- Linking $@"
	$(LD) $(LDFLAGS) -T $(LDFILE) -o $(TARGET) $(OBJECTS)

clean:
	@echo "-- Cleaning up"
	@rm -rf $(BLD_INNER) $(BUILD) $(ISODIR) $(TARGET) $(ISOFILE)

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

qemu:
	$(QEMU) $(QFLAGS)

hda:
	@dd if=/dev/urandom of=$(DISKIMG) bs=1M count=32

