# Toolchain
CC = x86_64-elf-gcc
LD = x86_64-elf-ld

# Flags
# -ffreestanding & -nostdlib: Minimalist environment
# -mcmodel=kernel: Matches higher-half address in linker script
CFLAGS = -m64 -march=x86-64 -O2 -ffreestanding -nostdlib \
         -mno-red-zone -fno-stack-protector -fno-pic -fno-pie \
         -mcmodel=kernel -Isrc -I. -Wall -mno-sse -mno-sse2 \
         -mno-mmx -mno-80387

LDFLAGS = -T linker.ld -static -nostdlib -z max-page-size=0x1000

# File/Directory Names
SRC_DIR = src
KERNEL  = kernel.elf

# Source and Object definitions
SRCS = $(shell find $(SRC_DIR) -name '*.c' -or -name '*.S') 
OBJS = $(patsubst %.c,%.o,$(patsubst %.S,%.o,$(SRCS)))

.PHONY: all clean

all: $(KERNEL)

# Link the kernel
$(KERNEL): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(KERNEL)

# Compile Assembly
$(SRC_DIR)/%.o: $(SRC_DIR)/%.S
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(KERNEL)
	rm -rf iso_root $(ISO_IMAGE)

ISO_IMAGE = image.iso

.PHONY: all clean iso

# Necessary for MacOS boot, optional 'make iso'
iso: $(KERNEL)
	mkdir -p iso_root/boot/
	cp $(KERNEL) iso_root/boot/
	xorriso -as mkisofs -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o $(ISO_IMAGE)
	@echo "ISO created: $(ISO_IMAGE)"
