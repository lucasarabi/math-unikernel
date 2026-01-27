#!/bin/bash

ISO_NAME="unikernel.iso"

if [ ! -f "$ISO_NAME" ]; then
    echo "ERROR: $ISO_NAME not found! Run build_iso.sh first."
    exit 1
fi

echo "Launching Math Unikernel in QEMU..."
echo "Note: Check this terminal for serial output ('S')."

qemu-system-x86_64 \
    -drive file="$ISO_NAME",format=raw \
    -serial stdio \
    -m 512M \
    -no-reboot \
    -no-shutdown \
