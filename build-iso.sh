#!/bin/bash
set -e

ISO_ROOT="iso_root"
KERNEL="kernel.elf"
ISO_NAME="unikernel.iso"

if [ ! -f "$KERNEL" ]; then
    echo "ERROR: $KERNEL not found! Run make first."
    exit 1
fi

echo "Staging $KERNEL"
cp "$KERNEL" "$ISO_ROOT/"

echo "Generating ISO..."
xorriso -as mkisofs -b limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        "$ISO_ROOT" -o "$ISO_NAME"

echo "Installing Limine BIOS MBR..."
limine bios-install "$ISO_NAME"

echo "Build Complete: $ISO_NAME"
