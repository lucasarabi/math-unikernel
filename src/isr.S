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
.section .text
.macro isr_no_err vector
.global isr_stub_\vector
isr_stub_\vector:
    pushq $0
    pushq $\vector
    jmp isr_common
.endm

.macro isr_err vector
.global isr_stub_\vector
isr_stub_\vector:
    pushq $\vector
    jmp isr_common
.endm

isr_no_err 0
isr_no_err 1
isr_no_err 2
isr_no_err 3
isr_no_err 4
isr_no_err 5
isr_no_err 6
isr_no_err 7
isr_err    8   # Double Fault pushes an error code
isr_no_err 9
isr_err    10  # Invalid TSS
isr_err    11  # Segment Not Present
isr_err    12  # Stack-Segment Fault
isr_err    13  # General Protection Fault
isr_err    14  # Page Fault
isr_no_err 15
isr_no_err 16
isr_err    17  # Alignment Check
isr_no_err 18
isr_no_err 19
isr_no_err 20
isr_err    21  # Control Protection Exception
isr_no_err 22
isr_no_err 23
isr_no_err 24
isr_no_err 25
isr_no_err 26
isr_no_err 27
isr_no_err 28
isr_err    29  # VMM Communication Exception
isr_err    30  # Security Exception
isr_no_err 31

# Initialize isr_stub_table
.section .data
.global isr_stub_table
.altmacro

# Helper macro to properly concatenate the number
.macro gen_quad num
    .quad isr_stub_\num
.endm

.macro gen_isr_table count
    .set i, 0
    .rept \count
        gen_quad %i
        .set i, i+1
    .endr
.endm

isr_stub_table:
    gen_isr_table 32
