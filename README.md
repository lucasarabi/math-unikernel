# Math Unikernel

A bare-metal x86-64 operating system kernel engineered exclusively for high-performance numerical computation. Unlike general-purpose operating systems, the unikernel exposes only the functionality required to receive, execute, and report the results of math workloads: memory management (physical and virtual), SIMD-accelerated math primitives, a lightweight Ethernet driver, a framebuffer display, and a serial console.

Workloads are small, position-independent C programs compiled against a thin **Kernel API SDK**. They are transferred to the running unikernel over a raw Ethernet link (custom EtherType `0x88B5`), executed in place, and then the unikernel returns to a polling state ready for the next workload.

---

## Table of Contents

1. [Directory Structure](#directory-structure)
2. [Component Descriptions](#component-descriptions)
3. [Installation](#installation)
   - [Prerequisites](#prerequisites)
   - [Building the OS](#building-the-os)
4. [Usage](#usage)
   - [Running in QEMU](#running-in-qemu)
   - [Running on Hardware](#running-on-hardware)
5. [VSCode Extension](#vscode-extension)
   - [Installing the Extension](#installing-the-extension)
   - [Developing a Workload](#developing-a-workload)
   - [Streaming a Workload](#streaming-a-workload)

---

## Directory Structure

```
math-unikernel/
├── src/                        ← All C and Assembly kernel source files
│   ├── headers/                ← Public header files for each subsystem
│   ├── nic_drivers/            ← Network interface card drivers
│   ├── kernel.c                ← Kernel entry point (kernel_main)
│   ├── boot.S                  ← Assembly boot stub
│   ├── gdt.c / gdt_load.S      ← Global Descriptor Table
│   ├── idt.c / isr.S           ← Interrupt Descriptor Table + ISRs
│   ├── pmm.c                   ← Physical Memory Manager
│   ├── vmm.c                   ← Virtual Memory Manager (huge pages)
│   ├── loader.c                ← Network workload loader (polling)
│   ├── network.c               ← Ethernet frame reception
│   ├── pci.c                   ← PCI bus scanner / NIC detection
│   ├── pic.c                   ← Programmable Interrupt Controller
│   ├── display.c               ← Framebuffer text renderer
│   ├── mathlib.c               ← AVX/FMA math primitives
│   ├── simd.S                  ← SIMD enable/CPUID helpers
│   ├── lib.c                   ← Minimal libc helpers (memset, etc.)
│   └── io.c                    ← Port I/O and serial driver
├── iso_root/                   ← Limine bootloader config + boot files
│   └── limine.cfg              ← Bootloader configuration
├── limine/                     ← Limine bootloader binaries (v7.x)
├── Makefile                    ← Compiles all sources → kernel.elf
├── linker.ld                   ← Linker script (higher-half kernel)
├── build-iso.sh                ← Wraps xorriso to produce bootable ISO
├── clean.sh                    ← Removes all build artifacts
├── qemu.sh                     ← Launches QEMU with correct flags
├── run_all.sh                  ← make + build-iso.sh + qemu.sh in one step
└── flash_drive.sh              ← Writes ISO to a physical USB drive
```

---

## Component Descriptions

The kernel is compiled with `x86_64-elf-gcc` using a freestanding, no-stdlib configuration. It boots via the Limine bootloader (v7.x), initializes hardware, and then enters an infinite poll → execute → poll loop.

| File | Description |
|---|---|
| `kernel.c` | Main kernel entry; hardware init sequence; state machine |
| `boot.S` | Assembly entry point called by Limine; sets up stack |
| `gdt.c` / `gdt_load.S` | 64-bit GDT setup |
| `idt.c` / `isr.S` | 64-bit IDT; interrupt service routines |
| `pmm.c` | Bitmap-based physical frame allocator |
| `vmm.c` | 4-level page table manager; huge-page allocator |
| `loader.c` | Receives workload size + payload over Ethernet frames |
| `network.c` | Raw Ethernet frame reception; interrupt-driven DMA |
| `pci.c` | PCI bus scan; locates NIC by class/vendor ID |
| `pic.c` | PIC remapping (IRQs 0–15 → vectors 32–47) |
| `display.c` | Pixel-perfect 8×8 font framebuffer text output |
| `mathlib.c` | AVX/FMA dot product, GEMM, SpMV (SSE+AVX enabled for this file only) |
| `simd.S` | Enables x87/SSE/AVX state in CR0/CR4/XCR0 |
| `lib.c` | `memset`, `memcpy`, `itoa`, `strlen` equivalents |
| `io.c` | `inb`/`outb`; 8250-compatible serial UART |
| `nic_drivers/rtl8139.c` | RTL8139 driver (QEMU default NIC) |
| `nic_drivers/i219.c` | Intel I219 driver (physical hardware) |

**Build artifacts:**
- `kernel.elf` — ELF binary produced by `make`
- `*.iso` — Bootable ISO produced by `build-iso.sh` (can be flashed to USB or booted in QEMU)

---

## Installation

### Prerequisites

| Tool | Purpose | Install |
|---|---|---|
| `x86_64-elf-gcc` / `x86_64-elf-ld` | Cross-compiler for bare-metal x86-64 | Arch: `pacman -S x86_64-elf-gcc` |
| `xorriso` | Builds the bootable ISO | Arch: `pacman -S xorriso` · Ubuntu: `apt install xorriso` |
| `qemu-system-x86_64` | Emulated testing | Arch: `pacman -S qemu-system-x86` · Ubuntu: `apt install qemu-system-x86` |
| Python 3 + Scapy | Workload streaming | `pip install scapy` (requires root or `CAP_NET_RAW`) |
| VSCode 1.115.0+ | Extension host | [code.visualstudio.com](https://code.visualstudio.com/) |
| Node.js 18 LTS+ | Build extension from source (optional) | [nodejs.org](https://nodejs.org/) |

---

### Building the OS

```bash
cd math_unikernel_os

# 1. Compile the kernel
make
# Produces: kernel.elf

# 2. Build the bootable ISO
./build-iso.sh
# Produces: math_unikernel.iso

# 3. Run in QEMU to verify
./qemu.sh
```

**Expected serial output after boot:**
```
[OK] Limine handshake
[OK] GDT initialized
[OK] IDT initialized
[OK] PMM initialized
[OK] VMM initialized
[OK] Display initialized
[OK] Serial driver initialized
[OK] Network controller found
[OK] Timer calibrated
[POLLING] Waiting for workload...
```

> **Note:** Emulated NIC drivers for QEMU are deprecated. The OS loop will not run to completion when emulated.

**Optional — flash to a physical USB drive (intended method)** (destructive, double-check the device path):
```bash
sudo ./flash_drive.sh /dev/sdX
```

**Convenience script** (runs all three steps at once):
```bash
./run_all.sh
```

---

## Usage

### Running in QEMU

```bash
cd math_unikernel_os
./qemu.sh
```

QEMU is configured with: no display window (serial → stdout), 512 MB RAM, one RTL8139 NIC on the user network (NAT). The kernel stays in the `POLLING` state until a valid workload is received. Press `Ctrl+C` to stop.

---

### Running on Hardware

**1. Flash the ISO to a USB drive** (destructive — double-check the device path):

```bash
sudo ./flash_drive.sh /dev/sdX
```

**2. Boot the target machine** from the USB drive. The kernel prints its initialization log over the serial port (COM1, 115200 8N1). Connect a serial cable or USB-to-serial adapter from the host to the target to see output.

**3. Note the MAC address** printed on boot:
```
[OK] Network controller found — MAC: de:ad:be:ef:00:01
```

Set this value as `mathUnikernel.targetMac` in the VSCode extension settings (or pass it directly to `send.py`).

**4. Connect the host and target** on the same Ethernet segment (direct cable or unmanaged switch). Set `mathUnikernel.interface` to the host NIC facing the target.

The kernel will stay in the `POLLING` state indefinitely until a workload is received. See the [VSCode Extension](#vscode-extension) section to stream one.

---

## VSCode Extension

GitHub: [math-unikernel-VSCode](https://github.com/lucasarabi/math-unikernel-VSCode)

A VSCode extension providing a sidebar panel ("MU Controls") with **Compile** and **Stream** commands to streamline the workload development loop.

| File | Description |
|---|---|
| `src/extension.ts` | Registers commands and the sidebar webview; activates on startup |
| `src/handlers.ts` | `compile` invokes `x86_64-elf-gcc` + `objcopy`; `stream` calls `send.py` |
| `src/sidebarProvider.ts` | Renders the MU Controls webview panel |
| `sdk/kernel_api.h` | Defines `KERNEL_API_ADDRESS`, the `kernel_api_t` struct, VMM flags, and the `MAIN` macro |
| `sdk/linker_script.ld` | Places `.entry` at address `0x0` for a flat binary with entry at offset 0 |
| `sdk/Makefile` | Builds `workload.c` → `workload.elf` → `workload.bin` |
| `python/send.py` | Scapy script that streams the workload binary over raw Ethernet |

**VSCode settings:**

| Setting | Description |
|---|---|
| `mathUnikernel.targetMac` | MAC address printed by the unikernel on boot |
| `mathUnikernel.interface` | Host network interface (e.g., `eno1`, `eth0`) |

---

### Installing the Extension

#### Option A — Install from pre-built `out/` directory

```bash
# Linux / macOS
cp -r math_unikernel_vscode_extension ~/.vscode/extensions/mu-0.0.1

# Windows (PowerShell)
Copy-Item -Recurse math_unikernel_vscode_extension "$env:USERPROFILE\.vscode\extensions\mu-0.0.1"
```

1. Restart VSCode.
2. Open **Extensions** (`Ctrl+Shift+X`) and search for `math-unikernel-VSCode` to confirm installation.
3. Configure via **File → Preferences → Settings** (`Ctrl+,`), search `Math Unikernel`:
   - `mathUnikernel.targetMac` — MAC address shown by the kernel on boot
   - `mathUnikernel.interface` — your host NIC (e.g., `eno1`, `eth0`)

#### Option B — Build from TypeScript source

```bash
cd math_unikernel_vscode_extension
npm install
npm run compile   # writes output to out/
```

Then follow Option A to install. To develop with hot-reload, open the folder in VSCode and press **F5** (uses `.vscode/launch.json`).

---

### Developing a Workload

1. Open your workload project folder in VSCode. The folder must contain `workload.c` and an `sdk/` subdirectory (`kernel_api.h`, `linker_script.ld`, `Makefile`). If `sdk/` is missing, a prompt will appear that will ask to create it in the folder root directory.
   
2. The **MU Controls** panel appears at the bottom of the Explorer sidebar with **Compile** and **Stream** buttons.

3. Write your workload using the `MAIN` macro as the entry point:

```c
#include "sdk/kernel_api.h"

MAIN int main(void) {
    kernel_api_t* api = (kernel_api_t*)KERNEL_API_ADDRESS;
    api->prints("Hello from workload!\n");
    return 0;
}
```

> The `MAIN` macro places `main()` in the `.entry` section at address `0x0` of the flat binary so the kernel can jump directly to it.

4. Click **Compile** (or `Ctrl+Shift+P` → `MU: Compile`). This runs the SDK Makefile and produces `workload.bin`.

5. With the unikernel running, click **Stream** (or `MU: Stream`). The extension calls `send.py` to transmit `workload.bin` to the configured MAC over the configured interface.

6. Observe output in the QEMU terminal or serial console.

---

### Streaming a Workload

The streaming protocol:
1. A handshake frame is sent with magic bytes `0x474F2121` (`"GO!!"`).
2. An 8-byte little-endian integer specifies the payload byte count.
3. The binary is chunked into 1486-byte payloads sent as raw Ethernet frames with EtherType `0x88B5`.

To stream manually without the VSCode extension:

```bash
sudo python3 math_unikernel_vscode_extension/python/send.py \
    workload.bin <target_mac> <interface>

# Example:
sudo python3 send.py workload.bin de:ad:be:ef:00:01 eno1
```

> Root privileges (or `CAP_NET_RAW`) are required for Scapy to send raw Ethernet frames.

---

## Credits

**font8x8_basic.h** — 8×8 monochrome bitmap font used by the framebuffer text renderer.
Originally by Marcel Sondaar / IBM (public domain VGA fonts); adapted by Daniel Hepper.
License: Public Domain.

**limine.h** — Limine bootloader protocol header.
Copyright © 2022–2024 mintsuki and contributors.
License: [BSD Zero Clause License](https://opensource.org/license/0bsd).
