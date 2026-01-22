#1/bin/bash

x86_64-elf-gcc -m64 -march=x86-64 -O2 -ffreestanding -nostdlib \
    -mno-red-zone -fno-stack-protector -fno-pic -fno-pie \
    -Isrc -I. \
    -T linker.ld src/boot.S src/kernel.c -o kernel.elf
