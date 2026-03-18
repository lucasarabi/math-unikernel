PHDRS {
    text PT_LOAD;
}

SECTIONS {
    . = 0x400000;

    .entry : {
        *(.entry)
    } :text

    .text : {
        *(.text)
    } :text 

    .rodata : {
        *(.rodata)
    } :text

    .data : {
        *(.data)
    } :text

    .bss : {
        *(.bss)
    } :text
}