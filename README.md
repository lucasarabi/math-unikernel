# math-unikernel

Project Plan: https://docs.google.com/document/d/11_AAv_jsCrgcUY2xzpXuPdycS8RdtfFbQYkHFuwx74A/edit?usp=sharing

### Quick Guide on shell scripts in project root:
- clean.sh
    -  Removes all build artifacts; this includes object files, ELF files, and ISOs.
- build_iso.sh
    -  Run after compiling with 'make'
    -  Invokes xorriso and Limine to build the ISO file and install the Limine bootloader
- qemu.sh
    -  Mostly for convenience; runs QEMU emulation of the OS on target system.
    -  Important QEMU configurations:
        -   System is completely headless; there is no framebuffer, hence the lack of display
        -   OS serial printer implementation prints data to serial port COM1. QEMU is printing to host system stdout.
        -   QEMU emulator has 512MB alloted for memory

Build system by running in the following order:
make -> ./build-iso.sh -> ./qemu.sh
Reset with ./clean.sh
