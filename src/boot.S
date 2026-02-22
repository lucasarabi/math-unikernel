/* Limine-compliant boot assembly for Math Unikernel - x86_64 */

# LIMINE BASE REVISION REQUEST
.section .requests, "a", @progbits
.balign 8
limine_base_revision:
    .quad 0xf95623d0d23a4821    # Magic Number 1
    .quad 0x27a1a039145b6951    # Magic Number 2
    .quad 0                     # Revision 0

# ALLOCATE
.section .bss
.balign 64
stack_bottom:
	.skip 32768
stack_top:

# POINT
.section .text
.global _start

_start:
	movq $stack_top, %rsp
	andq $-16, %rsp

	# TERMINATE
	xorq %rbp, %rbp

	
	# JUMP
	.extern kernel_main 
	call kernel_main

# SAFETY 
	halt:
	cli
	hlt
	jmp halt 

/* IDT/ISR Logic */

# utility for idt_init() in idt_mu.c
.section .text
.global load_idt
load_idt:
	lidt (%rdi) # fun fact: first argument in C function call is loaded in rdi register for x86_64
	ret

# Common bridge
.section .text
.extern exception_handler
isr_common:
	# Save 15 general-purpose registers
	pushq %rax; pushq %rbx; pushq %rcx; pushq %rdx
    pushq %rsi; pushq %rdi; pushq %rbp; pushq %r8
    pushq %r9;  pushq %r10; pushq %r11; pushq %r12
    pushq %r13; pushq %r14; pushq %r15

	# Pass current stack pointer as first arg to C 
	movq %rsp, %rdi
	call exception_handler

	# Restore snapshot in reverse order
    popq %r15; popq %r14; popq %r13; popq %r12
    popq %r11; popq %r10; popq %r9;  popq %r8
    popq %rbp; popq %rdi; popq %rsi; popq %rdx
    popq %rcx; popq %rbx; popq %rax

	addq $16, %rsp      # Clean up vector # and dummy error code
    iretq               # Hardware return to original math code


# ISR Stubs
# TODO -- Fix the sections below. They can, and should be written using a loop

.section .text

.global isr_stub_0;  isr_stub_0:  pushq $0; pushq $0;  jmp isr_common
.global isr_stub_1;  isr_stub_1:  pushq $0; pushq $1;  jmp isr_common
.global isr_stub_2;  isr_stub_2:  pushq $0; pushq $2;  jmp isr_common
.global isr_stub_3;  isr_stub_3:  pushq $0; pushq $3;  jmp isr_common
.global isr_stub_4;  isr_stub_4:  pushq $0; pushq $4;  jmp isr_common
.global isr_stub_5;  isr_stub_5:  pushq $0; pushq $5;  jmp isr_common
.global isr_stub_6;  isr_stub_6:  pushq $0; pushq $6;  jmp isr_common
.global isr_stub_7;  isr_stub_7:  pushq $0; pushq $7;  jmp isr_common
.global isr_stub_8;  isr_stub_8:  pushq $0; pushq $8;  jmp isr_common
.global isr_stub_9;  isr_stub_9:  pushq $0; pushq $9;  jmp isr_common
.global isr_stub_10; isr_stub_10: pushq $0; pushq $10; jmp isr_common
.global isr_stub_11; isr_stub_11: pushq $0; pushq $11; jmp isr_common
.global isr_stub_12; isr_stub_12: pushq $0; pushq $12; jmp isr_common
.global isr_stub_13; isr_stub_13: pushq $0; pushq $13; jmp isr_common
.global isr_stub_14; isr_stub_14: pushq $0; pushq $14; jmp isr_common
.global isr_stub_15; isr_stub_15: pushq $0; pushq $15; jmp isr_common
.global isr_stub_16; isr_stub_16: pushq $0; pushq $16; jmp isr_common
.global isr_stub_17; isr_stub_17: pushq $0; pushq $17; jmp isr_common
.global isr_stub_18; isr_stub_18: pushq $0; pushq $18; jmp isr_common
.global isr_stub_19; isr_stub_19: pushq $0; pushq $19; jmp isr_common
.global isr_stub_20; isr_stub_20: pushq $0; pushq $20; jmp isr_common
.global isr_stub_21; isr_stub_21: pushq $0; pushq $21; jmp isr_common
.global isr_stub_22; isr_stub_22: pushq $0; pushq $22; jmp isr_common
.global isr_stub_23; isr_stub_23: pushq $0; pushq $23; jmp isr_common
.global isr_stub_24; isr_stub_24: pushq $0; pushq $24; jmp isr_common
.global isr_stub_25; isr_stub_25: pushq $0; pushq $25; jmp isr_common
.global isr_stub_26; isr_stub_26: pushq $0; pushq $26; jmp isr_common
.global isr_stub_27; isr_stub_27: pushq $0; pushq $27; jmp isr_common
.global isr_stub_28; isr_stub_28: pushq $0; pushq $28; jmp isr_common
.global isr_stub_29; isr_stub_29: pushq $0; pushq $29; jmp isr_common
.global isr_stub_30; isr_stub_30: pushq $0; pushq $30; jmp isr_common
.global isr_stub_31; isr_stub_31: pushq $0; pushq $31; jmp isr_common

# Initialize isr_stub_table
.section .data
.global isr_stub_table
isr_stub_table:
    .quad isr_stub_0
    .quad isr_stub_1
    .quad isr_stub_2
    .quad isr_stub_3
    .quad isr_stub_4
    .quad isr_stub_5
    .quad isr_stub_6
    .quad isr_stub_7
    .quad isr_stub_8
    .quad isr_stub_9
    .quad isr_stub_10
    .quad isr_stub_11
    .quad isr_stub_12
    .quad isr_stub_13
    .quad isr_stub_14
    .quad isr_stub_15
    .quad isr_stub_16
    .quad isr_stub_17
    .quad isr_stub_18
    .quad isr_stub_19
    .quad isr_stub_20
    .quad isr_stub_21
    .quad isr_stub_22
    .quad isr_stub_23
    .quad isr_stub_24
    .quad isr_stub_25
    .quad isr_stub_26
    .quad isr_stub_27
    .quad isr_stub_28
    .quad isr_stub_29
    .quad isr_stub_30
    .quad isr_stub_31
