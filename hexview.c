/*
 * hexview.c - Standalone hexadecimal memory viewer
 *
 * author: luke8086
 * license: GPL-2
 */

#include <stdint.h>

#include "bios.h"
#include "util.h"

#define PAGE_ROWS  (TEXT_HEIGHT - 2)
#define PAGE_BYTES (PAGE_ROWS * 16)

#define ADDR_MAX   (0x100000 - PAGE_BYTES)

#define CL_BG      CL_BLUE
#define CL_FG      CL_LIGHT_GRAY

static void
fill_rect(int x, int y, int w, int h, unsigned char c, int fg, int bg)
{
    struct attr_char ch = {
        .ascii = c,
        .attr = (bg << 4) | fg
    };

    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            CHAR_AT(x + i, y + j) = ch;
        }
    }
}

static void
draw_box(int x, int y, int w, int h, int fg, int bg)
{
    fill_rect(x + 1, y, w - 2, 1, 0xc4, fg, bg);
    fill_rect(x + 1, y + h - 1, w - 2, 1, 0xc4, fg, bg);
    fill_rect(x, y + 1, 1, h - 2, 0xb3, fg, bg);
    fill_rect(x + w - 1, y + 1, 1, h - 2, 0xb3, fg, bg);
    fill_rect(x, y, 1, 1, 0xda, fg, bg);
    fill_rect(x + w - 1, y, 1, 1, 0xbf, fg, bg);
    fill_rect(x, y + h - 1, 1, 1, 0xc0, fg, bg);
    fill_rect(x + w - 1, y + h - 1, 1, 1, 0xd9, fg, bg);
}

static void
draw_hex(int x, int y, unsigned n, int len)
{
    for (; len; --len, n >>= 4)
        CHAR_AT(x + len - 1, y).ascii = "0123456789ABCDEF"[n & 0x0f];
}

static void
draw_hexdump(size_t addr)
{
    char GS_RELATIVE *ptr = 0;

    set_gs(addr >> 4);

    for (int y = 1; y < (1 + PAGE_ROWS); ++y) {
        draw_hex(1, y, addr, 6);

        for (int i = 0; i < 16; ++i, ++ptr, ++addr) {
            draw_hex(10 + i * 3 + i / 4, y, *ptr, 2);
            CHAR_AT(63 + i, y).ascii = *ptr;
        }
    }
}

void ENTRY_POINT
main(void)
{
    int32_t addr = 0x10000;

    set_fs(TEXT_MEM_SEG);

    toggle_cursor(0);

    fill_rect(0, 0, TEXT_WIDTH, PAGE_ROWS + 2, ' ', CL_FG, CL_BG);
    draw_box(0, 0, 8, PAGE_ROWS + 2, CL_FG, CL_BG);
    draw_box(8, 0, 54, PAGE_ROWS + 2, CL_FG, CL_BG);
    draw_box(62, 0, 18, PAGE_ROWS + 2, CL_FG, CL_BG);

    for (;;) {
        draw_hexdump(addr);

        switch (get_keystroke().scancode) {
            case SC_UP:   addr -= 16; break;
            case SC_DOWN: addr += 16; break;
            case SC_PGUP: addr -= PAGE_BYTES; break;
            case SC_PGDN: addr += PAGE_BYTES; break;
        }

        if (addr > ADDR_MAX) addr = ADDR_MAX;
        else if (addr < 0) addr = 0;
    }
}
