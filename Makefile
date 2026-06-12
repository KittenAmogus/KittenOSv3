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

# Custom stdlibs
LIBS_DIR  = src/libs
LIBS = $(shell find $(LIBS_DIR) -mindepth 1 -type d)
LIBS_INC = $(patsubst %,-I%,$(LIBS))

# Sources
SSOURCES = $(shell find src -name '*.s')
CSOURCES = $(shell find src -name '*.c')

# Objects
SOBJECTS = $(patsubst src/%.s,$(BUILD)/%.o,$(SSOURCES))
COBJECTS = $(patsubst src/%.c,$(BUILD)/%.o,$(CSOURCES))
OBJECTS = $(SOBJECTS) $(COBJECTS)

# Link config
LDFILE = src/link.ld

# Target
TARGET = kernel.elf

# ISO
ISODIR    = iso
ISOFILE   = os.iso
GRUBFILES = grub/grub.cfg

# BOCHS
BOCHS = bochs
BFLAGS = -f ./bochsrc.txt -q


.PHONY: all clean run

all: $(TARGET) iso

$(BUILD)/%.o: src/%.s | $(BUILD)
	@echo "-- Assembling $< -> $@"
	$(ASM) $(SFLAGS) $< -o $@

$(BUILD)/%.o: src/%.c | $(BUILD)
	@echo "-- Compiling $< -> $@"
	$(CC) $(CFLAGS) -c $< -o $@ $(LIBS_INC)

$(TARGET): $(OBJECTS)
	@echo "-- Linking $@"
	$(LD) $(LDFLAGS) -T $(LDFILE) -o $(TARGET) $(OBJECTS) $(LIBS_INC)

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

run:
	$(BOCHS) $(BFLAGS)

