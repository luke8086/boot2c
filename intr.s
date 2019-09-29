/*
 * intr.s - A generic function for triggering software interrupts
 *
 * author: luke8086
 * license: GPL-2
 */

    .code16

    .global intr

intr:
    push   %ebp
    mov    %esp, %ebp

    /* Preserve general-purpose registers */
    pushal

    /* Inject the interrupt number */
    mov    0x8(%ebp), %eax
    mov    %al, 2f + 0x01

    /* Just in case, invalidate the pre-fetch queue */
    jmp    1f
1:

    /* Save ebp, since it's also used as an input for BIOS */
    push   %ebp

    /* Load values from struct regs to the registers */
    mov    0xc(%ebp), %esi
    mov    0x00(%esi), %eax
    mov    0x04(%esi), %ebx
    mov    0x08(%esi), %ecx
    mov    0x0c(%esi), %edx
    mov    0x10(%esi), %ebp
    mov    0x14(%esi), %edi
    mov    0x18(%esi), %esi

    /* Trigger the interrupt */
2:  int    $0x0

    /* Restore ebp of our stack frame */
    pop    %ebp

    /* Save registers back to struct regs */
    mov    0xc(%ebp), %esi
    mov    %eax, 0x00(%esi)
    mov    %ebx, 0x04(%esi)
    mov    %ecx, 0x08(%esi)
    mov    %edx, 0x0c(%esi)
    pushfl
    popl   0x1c(%esi)

    /* Restore general-purpose registers */
    popal

    /* Just in case, clear the direction flag */
    cld

    pop    %ebp
    retl
