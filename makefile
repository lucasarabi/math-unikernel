# Toolchain
CC = x86_64-elf-gcc
LD = x86_64-elf-ld

# Flags
CFLAGS = -m64 -march=x86-64 -O2 -ffreestanding -nostdlib \
         -mno-red-zone -fno-stack-protector -fno-pic -fno-pie \
         -Isrc -I.

LDFLAGS = -T linker.ld -static -nostdlib -z max-page-size=0x1000

# File/Directory Names
SRC_DIR = src
ISO_DIR = iso_root
KERNEL = $(ISO_DIR)/kernel.elf
ISO = unikernel.iso

# Source and Object definitions
SRCS = $(SRC_DIR)/boot.S $(SRC_DIR)/kernel.c
OBJS = $(SRCS:.S=.o)
OBJS := $(OBJS:.c=.o)

# The default 'all' now builds the final ISO
all: $(ISO)

# 1. Compile Logic
%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 2. Linkage Logic: Place kernel.elf into iso_root
$(KERNEL): $(OBJS)
	mkdir -p $(ISO_DIR)
	$(LD) $(LDFLAGS) $(OBJS) -o $(KERNEL)

# 3. ISO Logic: Run xorriso and limine bios-install
$(ISO): $(KERNEL)
	# xorriso masters the filesystem structure
	xorriso -as mkisofs -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image \
		$(ISO_DIR) -o $(ISO)
	
	# Patch the MBR to make it bootable
	limine bios-install $(ISO)

# Utility Logic
run: $(ISO)
	qemu-system-x86_64 -cdrom $(ISO) -serial stdio

clean:
	rm -rf $(SRC_DIR)/*.o $(ISO_DIR)/kernel.elf $(ISO)
