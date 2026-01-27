#!/bin/bash

ISO_ROOT="iso_root"
SRC_DIR="src"
ISO_NAME="unikernel.iso"
KERNEL_BIN="kernel.elf"

echo "Cleaning Math Unikernel project..."

if [ -d "$SRC_DIR" ]; then
    echo "  -> Removing object files in $SRC_DIR"
    rm -f "$SRC_DIR"/*.o
fi

if [ -f "$KERNEL_BIN" ]; then
    echo "  -> Removing $KERNEL_BIN"
    rm -f "$KERNEL_BIN"
fi

if [ -d "$ISO_ROOT" ]; then
    echo "  -> Cleaning staged kernel from $ISO_ROOT"
    rm -f "$ISO_ROOT/$KERNEL_BIN"
fi

if [ -f "$ISO_NAME" ]; then
    echo "  -> Removing $ISO_NAME"
    rm -f "$ISO_NAME"
fi

echo "Clean complete."
