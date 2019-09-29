/*
 * util.c - Misc helpers
 *
 * author: luke8086
 * license: GPL-2
 */

#include "bios.h"
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

