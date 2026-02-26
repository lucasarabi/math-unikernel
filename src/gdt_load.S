# utility for gdt_init() in gdt_mu.c
.section .text
.global load_gdt
load_gdt:
    lgdt (%rdi)

    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    pushq $0x08
    lea .reload_CS(%rip), %rax
    pushq %rax
    lretq

.reload_CS:
    ret