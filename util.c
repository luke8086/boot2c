/*
 * util.c - BIOS interface and other basic helpers
 *
 * author: luke8086
 * license: GPL-2
 */

#include "util.h"

void
set_fs(uint16_t segment)
{
    __asm__ volatile ("mov %0, %%fs" :: "r"(segment) : "memory");
}

void
set_gs(uint16_t segment)
{
    __asm__ volatile ("mov %0, %%gs" :: "r"(segment) : "memory");
}

void *
memcpy(void *dest, const void *src, size_t n)
{
    unsigned char *srcb = (unsigned char *)src;
    unsigned char *destb = (unsigned char *)dest;

    while (n--) {
        *(destb++) = *(srcb++);
    }

    return dest;
}

void
put_string(char *s)
{
    while (*s)
        put_char(*s++);
}

uint16_t
rand(void)
{
    /* See https://en.wikipedia.org/wiki/Xorshift */

    static uint16_t seed = 0;

    if (seed == 0)
        seed = get_time();

    seed ^= (seed << 13);
    seed ^= (seed >> 9);
    seed ^= (seed << 7);

    return seed - 1;
}

void
toggle_cursor(int visible)
{
    struct regs regs = {
        .ah = 0x01,
        .ch = (visible ? 0x06 : 0x20 ),
        .cl = (visible ? 0x07 : 0x00 ),
    };

    intr(0x10, &regs);
}

void
move_cursor(int x, int y)
{
    struct regs regs = {
        .ah = 0x02,
        .bh = 0x00,
        .dh = y,
        .dl = x,
    };

    intr(0x10, &regs);
}

void
put_char(char ascii)
{
    struct regs regs = {
        .ah = 0x0e,
        .al = ascii,
        .bh = 0x00,
    };

    intr(0x10, &regs);
}

int
check_keystroke(void)
{
    struct regs regs = { .ah = 0x01 };

    intr(0x16, &regs);

    return !(regs.eflags & 0x0040);
}

struct keystroke
get_keystroke(void)
{
    struct regs regs = { .ah = 0x00 };

    intr(0x16, &regs);

    struct keystroke ret = {
        .scancode = regs.ah,
        .ascii    = regs.al,
    };

    return ret;
}

uint32_t
get_time(void)
{
    struct regs regs = { .ah = 0x00 };

    intr(0x1a, &regs);

    return ((regs.cx << 16) | regs.dx);
}
