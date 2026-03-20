CC      = gcc
CFLAGS  = -O2 -march=native -ffreestanding -nostdlib -fPIC
LDFLAGS = -Wl,-T,workload.ld -static
SRC     = workload.c
ELF     = workload.elf
TARGET  = workload.bin

.PHONY: all clean

all: $(TARGET)

$(ELF): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(ELF) $(SRC)

$(TARGET): $(ELF)
	objcopy -O binary $(ELF) $(TARGET)

clean:
	rm -f $(ELF) $(TARGET)
