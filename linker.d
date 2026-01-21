/* Tell the linker that our entry point is the _start label from boot.S */
ENTRY(_start)

/* Define the memory layout */
PHDRS
{
    text    PT_LOAD    FLAGS(0x05); /* Execute + Read */
    rodata  PT_LOAD    FLAGS(0x04); /* Read Only */
    data    PT_LOAD    FLAGS(0x06); /* Write + Read */
}

SECTIONS
{
    /* Limine expects the kernel to be loaded in the higher half */
    . = 0xffffffff80000000;

    /* 1. Limine Requests: Must be early so the bootloader finds them */
    .requests : {
        KEEP(*(.requests))
    } :rodata

    /* Move to the next 4KB page boundary */
    . = ALIGN(4096);

    /* 2. Executable Code */
    .text : {
        *(.text .text.*)
    } :text

    . = ALIGN(4096);

    /* 3. Read-Only Data (Constants) */
    .rodata : {
        *(.rodata .rodata.*)
    } :rodata

    . = ALIGN(4096);

    /* 4. Initialized Data */
    .data : {
        *(.data .data.*)
    } :data

    /* 5. Uninitialized Data (Where your stack lives) */
    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /* Discard unnecessary sections that bloat the binary */
    /DISCARD/ : {
        *(.eh_frame)
        *(.note .note.*)
    }
}
