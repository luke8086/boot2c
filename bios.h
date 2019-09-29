/*
 * bios.h - BIOS services
 *
 * author: luke8086
 * license: GPL-2
 */

#ifndef _BIOS_H_
#define _BIOS_H_

#include <stdint.h>

/*
 * Note: this library only covers a tiny subset of available functionality
 * of BIOS. To learn more, see http://stanislavs.org/helppc/idx_interrupt.html
 * and http://www.ctyme.com/rbrown.htm
 */


/*
 * Video services
 */

/* Text attributes */
enum {
    CL_BLACK,     CL_BLUE,          CL_GREEN,       CL_CYAN,
    CL_RED,       CL_MAGENTA,       CL_BROWN,       CL_LIGHT_GRAY,

    /* Bright colors require additional setup to be used for bg */
    CL_DARK_GRAY, CL_LIGHT_BLUE,    CL_LIGHT_GREEN, CL_LIGHT_CYAN,
    CL_LIGHT_RED, CL_LIGHT_MAGENTA, CL_YELLOW,      CL_WHITE
};

/* Toggle visibility of the cursor */
void toggle_cursor(int visible);

/* Move text cursor to the given location */
void move_cursor(int x, int y);

/* Write a single character at the cursor position */
void put_char(char);


/*
 * Direct access to the text-mode video memory.
 * Requires calling set_fs(TEXT_MEM_SEG);
 */

/* Default text-mode dimensions */
#define TEXT_WIDTH  80
#define TEXT_HEIGHT 25

/* Attributed character, a single item of the text-mode memory */
struct attr_char {
    uint8_t ascii;
    uint8_t attr;
};

/* Text-mode memory */
#define TEXT_MEM_SEG    0xb800
#define TEXT_MEM        ((struct attr_char FS_RELATIVE volatile *) 0x00)

/* Access a single character, assuming default screen dimensions */
#define CHAR_AT(x, y)   (TEXT_MEM[(y) * TEXT_WIDTH + (x)])


/*
 * Keyboard services
 */

/* Single keystroke in the keyboard buffer */
struct keystroke {
    char scancode;
    char ascii;
};

/* Keyboard scan codes */
enum {
    SC_ESC   = 0x01,
    SC_UP    = 0x48,
    SC_DOWN  = 0x50,
    SC_LEFT  = 0x4b,
    SC_RIGHT = 0x4d,
    SC_PGUP  = 0x49,
    SC_PGDN  = 0x51,
};

/* Check if a keystroke is ready in the keyboard buffer */
int check_keystroke(void);

/* Wait for, and return the next keystroke from the keyboard buffer */
struct keystroke get_keystroke(void);


/*
 * Clock services
 */

/* Get system time in 18.2Hz ticks since midnight */
uint32_t get_time(void);

#endif /* _BIOS_H_ */
