# Toolchain
CC = x86_64-elf-gcc
LD = x86_64-elf-ld

# Flags
# -ffreestanding & -nostdlib: Minimalist environment
# -mcmodel=kernel: Matches higher-half address in linker script
CFLAGS = -m64 -march=x86-64 -O2 -ffreestanding -nostdlib \
         -mno-red-zone -fno-stack-protector -fno-pic -fno-pie \
         -mcmodel=kernel -Isrc -I.

LDFLAGS = -T linker.ld -static -nostdlib -z max-page-size=0x1000

# File/Directory Names
SRC_DIR = src
KERNEL  = kernel.elf

# Source and Object definitions
SRCS = $(SRC_DIR)/boot.S $(SRC_DIR)/kernel.c
OBJS = $(SRC_DIR)/boot.o $(SRC_DIR)/kernel.o

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
