#!/bin/bash

make
./build-iso.sh
./qemu.sh
./clean.sh
