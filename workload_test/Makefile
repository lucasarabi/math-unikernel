CC = gcc

CFLAGS = -O2 -march=native -ffreestanding -nostdlib -fPIC
LDFLAGS = -Wl,-T,workload.ld

# Output
TARGET = workload.bin
SRC    = workload.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)