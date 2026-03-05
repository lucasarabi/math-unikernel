.section .text
.global enable_simd

enable_simd:
    pushq %rbx              

    movq %cr0, %rax
    andq $~(1<<2), %rax      
    orq $(1<<1), %rax        
    movq %rax, %cr0

    movq %cr4, %rax
    orq $(1<<9 | 1<<10), %rax
    
    pushq %rax              # Save our CR4 progress
    mov $1, %rax
    cpuid                   
    bt $26, %rcx            # Bit 26 is OSXSAVE support
    jnc .no_avx             # If not supported, skip AVX setup
    popq %rax               # Restore CR4 progress
    
    orq $(1<<18), %rax      # Now safely set Bit 18
    movq %rax, %cr4

    xorq %rcx, %rcx         
    xgetbv
    orq $7, %rax            
    xsetbv

    mov $(1<<5), %rax       # INIT SUCCESS flag (16)
    
    popq %rbx               
    ret

.no_avx:
    popq %rax
    movq %rax, %cr4         # Load CR4 without Bit 18

    mov $(1<<5), %rax       # INIT SUCCESS flag (16)
    
    popq %rbx               
    ret