
#include "rb_display.h"

#include <string.h>

/* Access to current display from rb_display.c */
extern rb_display *rb_get_current(void);

/* -------------------------------------------------------------------------- */
/*  Text buffer rendering                                                     */
/* -------------------------------------------------------------------------- */

void rb_display_text_set_immediate(int flag) {
    rb_get_current()->immediate_print = flag;
}

void rb_display_text_flush(void) {
    rb_display *d = rb_get_current();

    for (int row = 0; row < d->text_rows; row++) {
        for (int col = 0; col < d->text_cols; col++) {
            char ch = d->text_buffer[row * d->text_cols + col];
            byte paletteColor = d->text_colbuf[row * d->text_cols + col];

            int x = col * RET_FONT_WIDTH;
            int y = row * RET_FONT_HEIGHT;

            rb_display_render_draw_char(x, y, ch, 0, paletteColor);
        }
    }
}

static void rb_text_fast_clear(void) {
    rb_display *d = rb_get_current();
    memset(d->text_buffer, 0, d->text_cols * d->text_rows);
}

void rb_display_text_clear(void) {
    rb_display *d = rb_get_current();
    d->cursor_row = 0;
    d->cursor_col = 0;

    rb_text_fast_clear();
    rb_display_text_flush();
}

boolean rb_display_text_print_char(int row, int col, char ch, byte color) {
    rb_display *d = rb_get_current();

    byte code = (byte)ch;
    if (code >= RET_ESCAPE_BASE_INDEX_FOR_COLOR && code <= RET_ESCAPE_BASE_INDEX_FOR_COLOR + 15) {
        int new_color = (int)code - RET_ESCAPE_BASE_INDEX_FOR_COLOR;
        rb_display_set_fg_color(new_color);
        return false;
    }

    if (code == RET_ESCAPE_INVERT_ON) {
        rb_display_set_invert(true);
        return false;
    }

    if (code == RET_ESCAPE_INVERT_OFF) {
        rb_display_set_invert(false);
        return false;
    }

    if (col >= 0 && col < d->text_cols && row >= 0 && row < d->text_rows) {
        int offset = row * d->text_cols + col;

        d->text_buffer[offset] = ch;
        d->text_colbuf[offset] = color;

        if (d->immediate_print == 1) {
            int x = col * RET_FONT_WIDTH;
            int y = row * RET_FONT_HEIGHT;

            if (d->invert_mode) {
                rb_display_render_draw_char(x, y, ch, 1, color);
            }
            else {
                rb_display_render_draw_char(x, y, ch, 0, color);
            }
        }
    }

    return true;
}

char rb_display_text_get_char(int row, int col) {
    rb_display *d = rb_get_current();
    int offset = row * d->text_cols + col;
    return d->text_buffer[offset];
}

static boolean rb_text_print_char_internal(int row, int col, char ch) {
    return rb_display_text_print_char(row, col, ch, rb_display_get_fg_color());
}

void rb_display_text_scroll_up(void) {
    rb_display *d = rb_get_current();
    byte *source;
    byte *dest;

    for (int row = 1; row < d->text_rows; row++) {
        source = &d->text_buffer[row * d->text_cols];
        dest = &d->text_buffer[(row - 1) * d->text_cols];
        memcpy(dest, source, d->text_cols);
    }

    rb_display_render_scroll_up(RET_FONT_HEIGHT);

    /* Clear last line */
    dest = &d->text_buffer[(d->text_rows - 1) * d->text_cols];
    memset(dest, '\0', d->text_cols);
}

/* -------------------------------------------------------------------------- */
/*  Cursor handling                                                           */
/* -------------------------------------------------------------------------- */

void rb_display_cursor_show(boolean flag) {
    rb_get_current()->cursor_visible = flag;
}

void rb_display_cursor_draw(void) {
    rb_display *d = rb_get_current();

    if (!d->cursor_visible) {
        return;
    }

    int x = d->cursor_col * RET_FONT_WIDTH;
    int y = d->cursor_row * RET_FONT_HEIGHT;

    char ch = d->text_buffer[d->cursor_row * d->text_cols + d->cursor_col];

    rb_display_render_draw_char(x, y, ch, 1, RET_COLOR_CURSOR);
}

static void rb_cursor_reset_char(void) {
    rb_display *d = rb_get_current();

    int x = d->cursor_col * RET_FONT_WIDTH;
    int y = d->cursor_row * RET_FONT_HEIGHT;

    char ch = d->text_buffer[d->cursor_row * d->text_cols + d->cursor_col];
    byte paletteColor = d->text_colbuf[d->cursor_row * d->text_cols + d->cursor_col];

    rb_display_render_draw_char(x, y, ch, 0, paletteColor);
}

void rb_display_cursor_left(void) {
    rb_display *d = rb_get_current();
    rb_cursor_reset_char();

    d->cursor_col--;

    if (d->cursor_col < 0) {
        d->cursor_row--;
        d->cursor_col = d->text_cols - 1;
    }

    if (d->cursor_row < 0) {
        d->cursor_row = 0;
        d->cursor_col = 0;
    }

    rb_display_cursor_draw();
}

void rb_display_cursor_right(void) {
    rb_display *d = rb_get_current();
    rb_cursor_reset_char();

    d->cursor_col++;

    if (d->cursor_col > d->text_cols - 1) {
        d->cursor_row++;
        d->cursor_col = 0;
    }

    if (d->cursor_row > d->text_rows - 1) {
        rb_display_text_scroll_up();
        d->cursor_row = d->text_rows - 1;
    }

    rb_display_cursor_draw();
}

void rb_display_cursor_up(void) {
    rb_display *d = rb_get_current();
    rb_cursor_reset_char();

    d->cursor_row--;

    if (d->cursor_row < 0) {
        d->cursor_row = 0;
    }

    rb_display_cursor_draw();
}

void rb_display_cursor_down(void) {
    rb_display *d = rb_get_current();
    rb_cursor_reset_char();

    d->cursor_row++;

    if (d->cursor_row > d->text_rows - 1) {
        rb_display_text_scroll_up();
        d->cursor_row = d->text_rows - 1;
    }

    rb_display_cursor_draw();
}

void rb_display_cursor_set(int x, int y) {
    rb_display *d = rb_get_current();
    rb_cursor_reset_char();

    d->cursor_col = x;
    d->cursor_row = y;

    rb_display_cursor_draw();
}

static void rb_text_new_line(void) {
    rb_display *d = rb_get_current();
    rb_cursor_reset_char();

    d->cursor_row++;
    d->cursor_col = 0;

    if (d->cursor_row >= d->text_rows) {
        rb_display_text_scroll_up();
        d->cursor_row = d->text_rows - 1;
    }

    rb_display_cursor_draw();
}

void rb_display_cursor_blink(void) {
    rb_display *d = rb_get_current();

    d->cursor_cycles++;
    if (d->cursor_cycles < 25) {
        return;
    }

    d->cursor_cycles = 0;

    if (d->cursor_blink == 0) {
        rb_display_cursor_draw();
    }
    else {
        rb_cursor_reset_char();
    }

    d->cursor_blink = !d->cursor_blink;
}

void rb_display_cursor_delete_char(void) {
    rb_display *d = rb_get_current();
    rb_display_cursor_left();

    d->text_buffer[d->cursor_row * d->text_cols + d->cursor_col] = '\0';
}

/* -------------------------------------------------------------------------- */
/*  Text helper functions                                                     */
/* -------------------------------------------------------------------------- */

void rb_display_text_print_no_linebreak(char *str, int size) {
    rb_display *d = rb_get_current();
    int count = 0;

    while (*str != 0) {
        char ch = *str;

        rb_display_print_char_at(d->cursor_row, d->cursor_col, ch);

        d->cursor_col++;
        if (d->cursor_col >= d->text_cols) {
            d->cursor_col = d->text_cols - 1;
        }

        rb_display_cursor_draw();

        str++;
        count++;

        if (count == size) {
            return;
        }
    }
}

const char *rb_display_text_get_line_buffer(void) {
    return rb_get_current()->line_buffer;
}

/* -------------------------------------------------------------------------- */
/*  High-level text functions                                                 */
/* -------------------------------------------------------------------------- */

boolean rb_display_print_char_at(int row, int col, char ch) {
    return rb_text_print_char_internal(row, col, ch);
}

void rb_display_print_at(int row, int col, char *str) {
    while (*str != 0) {
        char ch = *str;
        rb_display_print_char_at(row, col, ch);
        col++;
        str++;
    }
}

void rb_display_print_char(char ch) {
    rb_display *d = rb_get_current();

    if (ch == '\n') {
        rb_text_new_line();
        return;
    }

    if (!rb_display_print_char_at(d->cursor_row, d->cursor_col, ch)) {
        return;
    }

    d->cursor_col++;
    if (d->cursor_col >= d->text_cols) {
        d->cursor_row++;
        d->cursor_col = 0;
    }

    if (d->cursor_row >= d->text_rows) {
        rb_display_text_scroll_up();
        d->cursor_row = d->text_rows - 1;
    }

    rb_display_cursor_draw();
}

void rb_display_print(char *str) {
    while (*str != 0) {
        char ch = *str;
        rb_display_print_char(ch);
        str++;
    }
}

void rb_display_print_line(char *str) {
    rb_display_print(str);
    rb_text_new_line();
}

void rb_display_print_newline(void) {
    rb_text_new_line();
}

void rb_display_print_palette(void) {
    byte fg = rb_display_get_fg_color();

    for (int i = 0; i < 128; i++) {
        rb_display_set_fg_color(i);
        rb_display_print_char((char)233);
    }

    rb_display_print_line("");

    rb_display_set_fg_color(fg);
}

void rb_display_show_charset(void) {
    for (int i = 0; i < 256; i++) {
        rb_display_print_char(i);
    }
}

