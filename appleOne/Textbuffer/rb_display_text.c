
#include "rb_display.h"

#include <string.h>

/* Access to display by index from rb_display.c */
extern rb_display *rb_get_display(int index);

/* -------------------------------------------------------------------------- */
/*  Text buffer rendering                                                     */
/* -------------------------------------------------------------------------- */

void rb_display_text_set_immediate(int display, int flag) {
    rb_get_display(display)->immediate_print = flag;
}

void rb_display_text_flush(int display) {
    rb_display *d = rb_get_display(display);

    for (int row = 0; row < d->text_rows; row++) {
        for (int col = 0; col < d->text_cols; col++) {
            char ch = d->text_buffer[row * d->text_cols + col];
            byte paletteColor = d->text_colbuf[row * d->text_cols + col];

            int x = col * RB_FONT_WIDTH;
            int y = row * RB_FONT_HEIGHT;

            rb_display_render_draw_char(display, x, y, ch, 0, paletteColor);
        }
    }
}

static void rb_text_fast_clear(rb_display *d) {
    memset(d->text_buffer, 0, d->text_cols * d->text_rows);
}

void rb_display_text_clear(int display) {
    rb_display *d = rb_get_display(display);
    d->cursor_row = 0;
    d->cursor_col = 0;

    rb_text_fast_clear(d);
    rb_display_text_flush(display);
}

boolean rb_display_text_print_char(int display, int row, int col, char ch, byte color) {
    rb_display *d = rb_get_display(display);

    byte code = (byte)ch;
    if (code >= RB_ESCAPE_BASE_INDEX_FOR_COLOR && code <= RB_ESCAPE_BASE_INDEX_FOR_COLOR + 15) {
        int new_color = (int)code - RB_ESCAPE_BASE_INDEX_FOR_COLOR;
        rb_display_set_fg_color(display, new_color);
        return false;
    }

    if (code == RB_ESCAPE_INVERT_ON) {
        rb_display_set_invert(display, true);
        return false;
    }

    if (code == RB_ESCAPE_INVERT_OFF) {
        rb_display_set_invert(display, false);
        return false;
    }

    if (col >= 0 && col < d->text_cols && row >= 0 && row < d->text_rows) {
        int offset = row * d->text_cols + col;

        d->text_buffer[offset] = ch;
        d->text_colbuf[offset] = color;

        if (d->immediate_print == 1) {
            int x = col * RB_FONT_WIDTH;
            int y = row * RB_FONT_HEIGHT;

            if (d->invert_mode) {
                rb_display_render_draw_char(display, x, y, ch, 1, color);
            }
            else {
                rb_display_render_draw_char(display, x, y, ch, 0, color);
            }
        }
    }

    return true;
}

char rb_display_text_get_char(int display, int row, int col) {
    rb_display *d = rb_get_display(display);
    int offset = row * d->text_cols + col;
    return d->text_buffer[offset];
}

static boolean rb_text_print_char_internal(int display, int row, int col, char ch) {
    return rb_display_text_print_char(display, row, col, ch, rb_display_get_fg_color(display));
}

void rb_display_text_scroll_up(int display) {
    rb_display *d = rb_get_display(display);
    byte *source;
    byte *dest;

    for (int row = 1; row < d->text_rows; row++) {
        source = &d->text_buffer[row * d->text_cols];
        dest = &d->text_buffer[(row - 1) * d->text_cols];
        memcpy(dest, source, d->text_cols);
    }

    rb_display_render_scroll_up(display, RB_FONT_HEIGHT);

    /* Clear last line */
    dest = &d->text_buffer[(d->text_rows - 1) * d->text_cols];
    memset(dest, '\0', d->text_cols);
}

/* -------------------------------------------------------------------------- */
/*  Cursor handling                                                           */
/* -------------------------------------------------------------------------- */

void rb_display_cursor_show(int display, boolean flag) {
    rb_get_display(display)->cursor_visible = flag;
}

void rb_display_cursor_draw(int display) {
    rb_display *d = rb_get_display(display);

    if (!d->cursor_visible) {
        return;
    }

    int x = d->cursor_col * RB_FONT_WIDTH;
    int y = d->cursor_row * RB_FONT_HEIGHT;

    char ch = d->text_buffer[d->cursor_row * d->text_cols + d->cursor_col];

    rb_display_render_draw_char(display, x, y, ch, 1, RB_COLOR_CURSOR);
}

static void rb_cursor_reset_char(int display) {
    rb_display *d = rb_get_display(display);

    int x = d->cursor_col * RB_FONT_WIDTH;
    int y = d->cursor_row * RB_FONT_HEIGHT;

    char ch = d->text_buffer[d->cursor_row * d->text_cols + d->cursor_col];
    byte paletteColor = d->text_colbuf[d->cursor_row * d->text_cols + d->cursor_col];

    rb_display_render_draw_char(display, x, y, ch, 0, paletteColor);
}

void rb_display_cursor_left(int display) {
    rb_display *d = rb_get_display(display);
    rb_cursor_reset_char(display);

    d->cursor_col--;

    if (d->cursor_col < 0) {
        d->cursor_row--;
        d->cursor_col = d->text_cols - 1;
    }

    if (d->cursor_row < 0) {
        d->cursor_row = 0;
        d->cursor_col = 0;
    }

    rb_display_cursor_draw(display);
}

void rb_display_cursor_right(int display) {
    rb_display *d = rb_get_display(display);
    rb_cursor_reset_char(display);

    d->cursor_col++;

    if (d->cursor_col > d->text_cols - 1) {
        d->cursor_row++;
        d->cursor_col = 0;
    }

    if (d->cursor_row > d->text_rows - 1) {
        rb_display_text_scroll_up(display);
        d->cursor_row = d->text_rows - 1;
    }

    rb_display_cursor_draw(display);
}

void rb_display_cursor_up(int display) {
    rb_display *d = rb_get_display(display);
    rb_cursor_reset_char(display);

    d->cursor_row--;

    if (d->cursor_row < 0) {
        d->cursor_row = 0;
    }

    rb_display_cursor_draw(display);
}

void rb_display_cursor_down(int display) {
    rb_display *d = rb_get_display(display);
    rb_cursor_reset_char(display);

    d->cursor_row++;

    if (d->cursor_row > d->text_rows - 1) {
        rb_display_text_scroll_up(display);
        d->cursor_row = d->text_rows - 1;
    }

    rb_display_cursor_draw(display);
}

void rb_display_cursor_set(int display, int x, int y) {
    rb_cursor_reset_char(display);

    rb_display *d = rb_get_display(display);
    d->cursor_col = x;
    d->cursor_row = y;

    rb_display_cursor_draw(display);
}

static void rb_text_new_line(int display) {
    rb_display *d = rb_get_display(display);
    rb_cursor_reset_char(display);

    d->cursor_row++;
    d->cursor_col = 0;

    if (d->cursor_row >= d->text_rows) {
        rb_display_text_scroll_up(display);
        d->cursor_row = d->text_rows - 1;
    }

    rb_display_cursor_draw(display);
}

void rb_display_cursor_blink(int display) {
    rb_display *d = rb_get_display(display);

    d->cursor_cycles++;
    if (d->cursor_cycles < 25) {
        return;
    }

    d->cursor_cycles = 0;

    if (d->cursor_blink == 0) {
        rb_display_cursor_draw(display);
    }
    else {
        rb_cursor_reset_char(display);
    }

    d->cursor_blink = !d->cursor_blink;
}

void rb_display_cursor_delete_char(int display) {
    rb_display *d = rb_get_display(display);
    rb_display_cursor_left(display);

    d->text_buffer[d->cursor_row * d->text_cols + d->cursor_col] = '\0';
}

/* -------------------------------------------------------------------------- */
/*  Text helper functions                                                     */
/* -------------------------------------------------------------------------- */

void rb_display_text_print_no_linebreak(int display, char *str, int size) {
    rb_display *d = rb_get_display(display);
    int count = 0;

    while (*str != 0) {
        char ch = *str;

        rb_display_print_char_at(display, d->cursor_row, d->cursor_col, ch);

        d->cursor_col++;
        if (d->cursor_col >= d->text_cols) {
            d->cursor_col = d->text_cols - 1;
        }

        rb_display_cursor_draw(display);

        str++;
        count++;

        if (count == size) {
            return;
        }
    }
}

const char *rb_display_text_get_line_buffer(int display) {
    return rb_get_display(display)->line_buffer;
}

/* -------------------------------------------------------------------------- */
/*  High-level text functions                                                 */
/* -------------------------------------------------------------------------- */

boolean rb_display_print_char_at(int display, int row, int col, char ch) {
    return rb_text_print_char_internal(display, row, col, ch);
}

void rb_display_print_at(int display, int row, int col, char *str) {
    while (*str != 0) {
        char ch = *str;
        rb_display_print_char_at(display, row, col, ch);
        col++;
        str++;
    }
}

void rb_display_print_char(int display, char ch) {
    rb_display *d = rb_get_display(display);

    if (ch == '\n') {
        rb_text_new_line(display);
        return;
    }

    if (!rb_display_print_char_at(display, d->cursor_row, d->cursor_col, ch)) {
        return;
    }

    d->cursor_col++;
    if (d->cursor_col >= d->text_cols) {
        d->cursor_row++;
        d->cursor_col = 0;
    }

    if (d->cursor_row >= d->text_rows) {
        rb_display_text_scroll_up(display);
        d->cursor_row = d->text_rows - 1;
    }

    rb_display_cursor_draw(display);
}

void rb_display_print(int display, char *str) {
    while (*str != 0) {
        char ch = *str;
        rb_display_print_char(display, ch);
        str++;
    }
}

void rb_display_print_line(int display, char *str) {
    rb_display_print(display, str);
    rb_text_new_line(display);
}

void rb_display_print_newline(int display) {
    rb_text_new_line(display);
}

void rb_display_print_palette(int display) {
    byte fg = rb_display_get_fg_color(display);

    for (int i = 0; i < 128; i++) {
        rb_display_set_fg_color(display, i);
        rb_display_print_char(display, (char)233);
    }

    rb_display_print_line(display, "");

    rb_display_set_fg_color(display, fg);
}

void rb_display_show_charset(int display) {
    for (int i = 0; i < 256; i++) {
        rb_display_print_char(display, i);
    }
}
