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
