/*
 * app.c - Sample standalone C app for x86
 *
 * author: luke8086
 * license: GPL-2
 */

#include "util.h"

/* Use 2 columns per 1 row, to achieve square blocks */
#define WIDTH  (TEXT_WIDTH / 2)
#define HEIGHT (TEXT_HEIGHT)

/* Duration of a single frame in ticks of the system timer */
#define FRAME_TICKS 3

struct coords {
    int x, y;
};

struct body {
    struct coords coords[WIDTH * HEIGHT];
    struct coords *head;
    struct coords *tail;
    int grow;
};

enum dir {
    DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT
};

/* Block types correpond to their foreground colors */
enum block {
    PT_FLOOR = CL_BLACK,
    PT_WALL  = CL_LIGHT_CYAN,
    PT_SNAKE = CL_YELLOW,
    PT_FRUIT = CL_LIGHT_MAGENTA
};

static void
set_block(int x, int y, enum block block)
{
    struct attr_char ch = { .ascii = 0xdb, .attr = block };
    CHAR_AT(x * 2, y) = CHAR_AT(x * 2 + 1, y) = ch;
}

static enum block
check_block(struct coords c)
{
    return CHAR_AT(c.x * 2, c.y).attr;
}

static void
fill_rect(int x, int y, int w, int h, enum block block)
{
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            set_block(x + i, y + j, block);
        }
    }
}

static void
draw_board(void)
{
    fill_rect(0, 0, WIDTH, HEIGHT, PT_WALL);
    fill_rect(1, 1, WIDTH - 2, HEIGHT - 2, PT_FLOOR);
    fill_rect((WIDTH - 6) / 2, 0, 6, 1, PT_FLOOR);
    fill_rect((WIDTH - 6) / 2, HEIGHT - 1, 6, 1, PT_FLOOR);
    fill_rect(0, (HEIGHT - 5) / 2, 1, 5, PT_FLOOR);
    fill_rect(WIDTH - 1, (HEIGHT - 5) / 2, 1, 5, PT_FLOOR);
}

static void
add_fruit(void) {
    struct coords c;

    do {
        c.x = rand() % WIDTH;
        c.y = rand() % HEIGHT;
    } while(check_block(c) != PT_FLOOR);

    set_block(c.x, c.y, PT_FRUIT);
}

static void
handle_kbd(enum dir prev_dir, enum dir *next_dir) {
    char sc = check_keystroke() ? (get_keystroke().scancode) : 0;

    if (sc == SC_UP && prev_dir != DIR_DOWN) *next_dir = DIR_UP;
    else if (sc == SC_DOWN && prev_dir != DIR_UP) *next_dir = DIR_DOWN;
    else if (sc == SC_LEFT && prev_dir != DIR_RIGHT) *next_dir = DIR_LEFT;
    else if (sc == SC_RIGHT && prev_dir != DIR_LEFT) *next_dir = DIR_RIGHT;
}

static void
init_body(struct body *body)
{
    body->coords[0].x = 16;
    body->coords[0].y = 12;
    body->head = body->tail = body->coords;
    body->grow = 8;
}

static struct coords
move_head(struct coords head, enum dir dir)
{
    switch (dir) {
    case DIR_UP:    head.y--; break;
    case DIR_DOWN:  head.y++; break;
    case DIR_LEFT:  head.x--; break;
    case DIR_RIGHT: head.x++; break;
    }

    head.x = (head.x + WIDTH) % WIDTH;
    head.y = (head.y + HEIGHT) % HEIGHT;

    return head;
}

static void
move_snake(struct body *s, struct coords next_head)
{
    if (s->grow) {
        ++s->tail;
        --s->grow;
    } else {
        set_block(s->tail->x, s->tail->y, PT_FLOOR);
    }

    for (struct coords *c = s->tail; c != s->head; --c) {
        *c = *(c - 1);
    }

    *(s->head) = next_head;

    set_block(s->head->x, s->head->y, PT_SNAKE);
}

void ENTRY_POINT
main(void)
{
    set_fs(TEXT_MEM_SEG);

    put_string("\r\nHello World! This is a standalone x86 app written in C\r\n", CL_LIGHT_CYAN);
    put_string("\r\nPress any key to play a game...\r\n", CL_LIGHT_GREEN);

    (void)get_keystroke();

    for (;;) {
        struct body body;
        enum dir prev_dir = DIR_RIGHT;
        enum dir next_dir = DIR_RIGHT;

        init_body(&body);
        draw_board();
        add_fruit();

        for (;;) {
            /* TODO: Handle midnight overflow */
            uint32_t frame_end = get_time() + FRAME_TICKS;

            struct coords next_head = move_head(*body.head, next_dir);
            enum block next_block = check_block(next_head);

            if (next_block == PT_FRUIT)
                body.grow += 2;
            else if (next_block != PT_FLOOR)
                break;

            move_snake(&body, next_head);

            if (next_block == PT_FRUIT)
                add_fruit();

            prev_dir = next_dir;

            do handle_kbd(prev_dir, &next_dir);
            while (get_time() < frame_end);
        }
    }
}
