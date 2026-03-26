#!/bin/bash

ISO_NAME="unikernel.iso"
DISK_NAME="/dev/sda"

sudo dd if=$ISO_NAME \
        of=$DISK_NAME \
        bs=4M \
        status=progress

sync