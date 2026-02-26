.section .text
.global enable_simd

enable_simd:
    # 1. CR0: Standard legacy math setup
    movq %cr0, %rax
    andq $~(1<<2), %rax      
    orq $(1<<1), %rax        
    movq %rax, %cr0

    # 2. CR4: Enable SSE and tell the CPU we WANT to use OSXSAVE
    movq %cr4, %rax
    orq $(1<<9 | 1<<10), %rax
    
    # 3. THE CRUCIAL CHECK: Does this CPU even support XSAVE?
    # We check CPUID leaf 1, ECX bit 26
    pushq %rax          # Save our CR4 progress
    mov $1, %rax
    cpuid
    bt $26, %rcx        # Bit 26 is OSXSAVE support
    jnc .no_avx         # If not supported, skip AVX setup to avoid stall
    popq %rax           # Restore CR4 progress
    
    orq $(1<<18), %rax  # Now safely set Bit 18
    movq %rax, %cr4

    # 4. XCR0: Now the gate is officially open
    xorq %rcx, %rcx     # Cleaner way to set RCX to 0
    xgetbv
    orq $7, %rax        # 7 is (1 | 1<<1 | 1<<2)
    xsetbv
    ret

.no_avx:
    popq %rax
    movq %rax, %cr4     # Load CR4 without Bit 18
    ret