OUTPUT_FORMAT(elf64-x86-64)

ENTRY(_start)

PHDRS
{
    text    PT_LOAD    FLAGS(0x05); /* Execute + Read */
    rodata  PT_LOAD    FLAGS(0x04); /* Read Only */
    data    PT_LOAD    FLAGS(0x06); /* Write + Read */
}

SECTIONS
{
    . = 0xffffffff80000000;

    .rodata : {
        KEEP(*(.requests))
        *(.rodata .rodata.*)
    } :rodata

    . = ALIGN(4096);

    .text : {
        *(.text .text.*)
    } :text

    . = ALIGN(4096);

    .data : {
        *(.data .data.*)
    } :data

    .bss : {
        *(.bss .bss.*)
        *(COMMON)
    } :data

    /DISCARD/ : {
        *(.eh_frame)
        *(.note .note.*)
    }
}
