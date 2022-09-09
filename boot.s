/*
 * boot.s - Minimal bootloader for USB drives
 *
 * author: luke8086
 * license: GPL-2
 */

    .code16
    
    /* Setup code, data and stack segments at 0x10000 - 0x1ffff */
    cli
    mov    $0x1000, %ax
    mov    %ax, %ds
    mov    %ax, %es
    mov    %ax, %ss
    mov    $0xfff0, %sp
    sti

    /* Load our program from disk to RAM using INT 13h, AH=02h */
    mov    $0x027f, %ax
    mov    $0x0000, %bx
    mov    $0x0003, %cx
    mov    $0x00, %dh
    int    $0x13

    /* Jump to the program */
    ljmp   $0x1000, $0x0
    
    /* MBR partition table with a single bootable partition */
    .org 0x01be
    .byte 0x80, 0x00, 0x02, 0x00
    .byte 0x01, 0x00, 0x3f, 0x00
    .long 0x01, 0x7f

    /* Magic number designating a boot loader */
    .org 0x01fe
    .byte 0x55, 0xaa
