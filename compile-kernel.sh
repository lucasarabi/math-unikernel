#!/bin/bash

x86_64-elf-gcc -T linker.ld -o kernel.elf src/boot.S src/kernel.c \
    -ffreestanding -nostdlib -mno-red-zone \
    -fno-stack-protector -fno-pic -fno-pie \
    -m64 -march=x86-64 -O2 -z max-page-size=0x1000
