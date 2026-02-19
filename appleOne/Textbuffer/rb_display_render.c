
#include "rb_display.h"
#include "ret_postprocess.h"

#include <stdlib.h>
#include <string.h>

/* Access to current display from rb_display.c */
extern rb_display *rb_get_current(void);

extern void ret_render_frame(byte *data, int index);
extern byte ret_font_apple2[];

/* -------------------------------------------------------------------------- */
/*  Vector font table                                                         */
/* -------------------------------------------------------------------------- */

static const byte rb_vecfont[128][8] = {
    ['0' - 0x20] = { P(0,0), P(8,0), P(8,12), P(0,12), P(0,0), P(8,12), FONT_LAST },
    ['1' - 0x20] = { P(4,0), P(4,12), P(3,10), FONT_LAST },
    ['2' - 0x20] = { P(0,12), P(8,12), P(8,7), P(0,5), P(0,0), P(8,0), FONT_LAST },
    ['3' - 0x20] = { P(0,12), P(8,12), P(8,0), P(0,0), FONT_UP, P(0,6), P(8,6), FONT_LAST },
    ['4' - 0x20] = { P(0,12), P(0,6), P(8,6), FONT_UP, P(8,12), P(8,0), FONT_LAST },
    ['5' - 0x20] = { P(0,0), P(8,0), P(8,6), P(0,7), P(0,12), P(8,12), FONT_LAST },
    ['6' - 0x20] = { P(0,12), P(0,0), P(8,0), P(8,5), P(0,7), FONT_LAST },
    ['7' - 0x20] = { P(0,12), P(8,12), P(8,6), P(4,0), FONT_LAST },
    ['8' - 0x20] = { P(0,0), P(8,0), P(8,12), P(0,12), P(0,0), FONT_UP, P(0,6), P(8,6), },
    ['9' - 0x20] = { P(8,0), P(8,12), P(0,12), P(0,7), P(8,5), FONT_LAST },
    [' ' - 0x20] = { FONT_LAST },
    ['.' - 0x20] = { P(3,0), P(4,0), FONT_LAST },
    [',' - 0x20] = { P(2,0), P(4,2), FONT_LAST },
    ['-' - 0x20] = { P(2,6), P(6,6), FONT_LAST },
    ['+' - 0x20] = { P(1,6), P(7,6), FONT_UP, P(4,9), P(4,3), FONT_LAST },
    ['!' - 0x20] = { P(4,0), P(3,2), P(5,2), P(4,0), FONT_UP, P(4,4), P(4,12), FONT_LAST },
    ['#' - 0x20] = { P(0,4), P(8,4), P(6,2), P(6,10), P(8,8), P(0,8), P(2,10), P(2,2) },
    ['^' - 0x20] = { P(2,6), P(4,12), P(6,6), FONT_LAST },
    ['=' - 0x20] = { P(1,4), P(7,4), FONT_UP, P(1,8), P(7,8), FONT_LAST },
    ['*' - 0x20] = { P(0,0), P(4,12), P(8,0), P(0,8), P(8,8), P(0,0), FONT_LAST },
    ['_' - 0x20] = { P(0,0), P(8,0), FONT_LAST },
    ['/' - 0x20] = { P(0,0), P(8,12), FONT_LAST },
    ['\\' - 0x20] = { P(0,12), P(8,0), FONT_LAST },
    ['@' - 0x20] = { P(8,4), P(4,0), P(0,4), P(0,8), P(4,12), P(8,8), P(4,4), P(3,6) },
    ['$' - 0x20] = { P(6,2), P(2,6), P(6,10), FONT_UP, P(4,12), P(4,0), FONT_LAST },
    ['&' - 0x20] = { P(8,0), P(4,12), P(8,8), P(0,4), P(4,0), P(8,4), FONT_LAST },
    ['[' - 0x20] = { P(6,0), P(2,0), P(2,12), P(6,12), FONT_LAST },
    [']' - 0x20] = { P(2,0), P(6,0), P(6,12), P(2,12), FONT_LAST },
    ['(' - 0x20] = { P(6,0), P(2,4), P(2,8), P(6,12), FONT_LAST },
    [')' - 0x20] = { P(2,0), P(6,4), P(6,8), P(2,12), FONT_LAST },
    ['{' - 0x20] = { P(6,0), P(4,2), P(4,10), P(6,12), FONT_UP, P(2,6), P(4,6), FONT_LAST },
    ['}' - 0x20] = { P(4,0), P(6,2), P(6,10), P(4,12), FONT_UP, P(6,6), P(8,6), FONT_LAST },
    ['%' - 0x20] = { P(0,0), P(8,12), FONT_UP, P(2,10), P(2,8), FONT_UP, P(6,4), P(6,2) },
    ['<' - 0x20] = { P(6,0), P(2,6), P(6,12), FONT_LAST },
    ['>' - 0x20] = { P(2,0), P(6,6), P(2,12), FONT_LAST },
    ['|' - 0x20] = { P(4,0), P(4,5), FONT_UP, P(4,6), P(4,12), FONT_LAST },
    [':' - 0x20] = { P(4,9), P(4,7), FONT_UP, P(4,5), P(4,3), FONT_LAST },
    [';' - 0x20] = { P(4,9), P(4,7), FONT_UP, P(4,5), P(1,2), FONT_LAST },
    ['"' - 0x20] = { P(2,10), P(2,6), FONT_UP, P(6,10), P(6,6), FONT_LAST },
    ['\'' - 0x20] = { P(2,6), P(6,10), FONT_LAST },
    ['`' - 0x20] = { P(2,10), P(6,6), FONT_LAST },
    ['~' - 0x20] = { P(0,4), P(2,8), P(6,4), P(8,8), FONT_LAST },
    ['?' - 0x20] = { P(0,8), P(4,12), P(8,8), P(4,4), FONT_UP, P(4,1), P(4,0), FONT_LAST },
    ['A' - 0x20] = { P(0,0), P(0,8), P(4,12), P(8,8), P(8,0), FONT_UP, P(0,4), P(8,4) },
    ['B' - 0x20] = { P(0,0), P(0,12), P(4,12), P(8,10), P(4,6), P(8,2), P(4,0), P(0,0) },
    ['C' - 0x20] = { P(8,0), P(0,0), P(0,12), P(8,12), FONT_LAST },
    ['D' - 0x20] = { P(0,0), P(0,12), P(4,12), P(8,8), P(8,4), P(4,0), P(0,0), FONT_LAST },
    ['E' - 0x20] = { P(8,0), P(0,0), P(0,12), P(8,12), FONT_UP, P(0,6), P(6,6), FONT_LAST },
    ['F' - 0x20] = { P(0,0), P(0,12), P(8,12), FONT_UP, P(0,6), P(6,6), FONT_LAST },
    ['G' - 0x20] = { P(6,6), P(8,4), P(8,0), P(0,0), P(0,12), P(8,12), FONT_LAST },
    ['H' - 0x20] = { P(0,0), P(0,12), FONT_UP, P(0,6), P(8,6), FONT_UP, P(8,12), P(8,0) },
    ['I' - 0x20] = { P(0,0), P(8,0), FONT_UP, P(4,0), P(4,12), FONT_UP, P(0,12), P(8,12) },
    ['J' - 0x20] = { P(0,4), P(4,0), P(8,0), P(8,12), FONT_LAST },
    ['K' - 0x20] = { P(0,0), P(0,12), FONT_UP, P(8,12), P(0,6), P(6,0), FONT_LAST },
    ['L' - 0x20] = { P(8,0), P(0,0), P(0,12), FONT_LAST },
    ['M' - 0x20] = { P(0,0), P(0,12), P(4,8), P(8,12), P(8,0), FONT_LAST },
    ['N' - 0x20] = { P(0,0), P(0,12), P(8,0), P(8,12), FONT_LAST },
    ['O' - 0x20] = { P(0,0), P(0,12), P(8,12), P(8,0), P(0,0), FONT_LAST },
    ['P' - 0x20] = { P(0,0), P(0,12), P(8,12), P(8,6), P(0,5), FONT_LAST },
    ['Q' - 0x20] = { P(0,0), P(0,12), P(8,12), P(8,4), P(0,0), FONT_UP, P(4,4), P(8,0) },
    ['R' - 0x20] = { P(0,0), P(0,12), P(8,12), P(8,6), P(0,5), FONT_UP, P(4,5), P(8,0) },
    ['S' - 0x20] = { P(0,2), P(2,0), P(8,0), P(8,5), P(0,7), P(0,12), P(6,12), P(8,10) },
    ['T' - 0x20] = { P(0,12), P(8,12), FONT_UP, P(4,12), P(4,0), FONT_LAST },
    ['U' - 0x20] = { P(0,12), P(0,2), P(4,0), P(8,2), P(8,12), FONT_LAST },
    ['V' - 0x20] = { P(0,12), P(4,0), P(8,12), FONT_LAST },
    ['W' - 0x20] = { P(0,12), P(2,0), P(4,4), P(6,0), P(8,12), FONT_LAST },
    ['X' - 0x20] = { P(0,0), P(8,12), FONT_UP, P(0,12), P(8,0), FONT_LAST },
    ['Y' - 0x20] = { P(0,12), P(4,6), P(8,12), FONT_UP, P(4,6), P(4,0), FONT_LAST },
    ['Z' - 0x20] = { P(0,12), P(8,12), P(0,0), P(8,0), FONT_UP, P(2,6), P(6,6), FONT_LAST },
    /* Special chars from 65 to 96 */
    [65] = { P(0,0), P(0,12), P(7,12), P(7,0), P(0,0), FONT_LAST },
    [66] = { P(0,0), P(4,12), P(8,0), P(0,0), FONT_LAST },
    [67] = { P(0,6), P(8,6), P(8,7), P(0,7), FONT_LAST },
};

/* -------------------------------------------------------------------------- */
/*  Helper                                                                    */
/* -------------------------------------------------------------------------- */

static int rb_getbit(byte b, byte number) {
    number = 8 - number;
    return (b >> number) & 1;
}

/* -------------------------------------------------------------------------- */
/*  Dirty bit                                                                 */
/* -------------------------------------------------------------------------- */

static void rb_enable_dirty(void) {
    rb_get_current()->dirty = 1;
}

/* -------------------------------------------------------------------------- */
/*  Pixel drawing (internal)                                                  */
/* -------------------------------------------------------------------------- */

static void rb_set_pixel_internal(int x, int y, int r, int g, int b) {
    rb_display *d = rb_get_current();
    int width = d->pixel_width;
    int height = d->pixel_height;

    if (x >= 0 && x < width && y >= 0 && y < height) {
        int offset = (x + y * width) * 4;

        d->pixel_data[offset] = r;
        d->pixel_data[offset + 1] = g;
        d->pixel_data[offset + 2] = b;
        d->pixel_data[offset + 3] = 255;
    }

    rb_enable_dirty();
}

static void rb_set_pixel(int x, int y, byte palette_color, byte brightness) {
    ret_color fg = RETPaletteGetColorWithBrightness(palette_color, brightness);
    rb_set_pixel_internal(x, y, fg.r, fg.g, fg.b);
}

/* -------------------------------------------------------------------------- */
/*  Line drawing (internal)                                                   */
/* -------------------------------------------------------------------------- */

static void rb_draw_line_internal(int x1, int y1, int x2, int y2,
                                  byte palette_color, byte brightness) {
    rb_display *d = rb_get_current();
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

    float xInc = dx / (float)steps;
    float yInc = dy / (float)steps;

    float x = x1;
    float y = y1;

    boolean draw = true;
    int count = 0;

    for (int i = 0; i <= steps; i++) {
        if (d->mode_dotted > 0) {
            if (draw) {
                rb_set_pixel(x, y, palette_color, brightness);
            }

            count++;

            if (count == d->mode_dotted) {
                count = 0;
                draw = !draw;
            }
        }
        else {
            rb_set_pixel(x, y, palette_color, brightness);
        }

        x += xInc;
        y += yInc;
    }
}

/* -------------------------------------------------------------------------- */
/*  Vector font drawing (internal)                                            */
/* -------------------------------------------------------------------------- */

static int rb_vec_cursor_x = 0;
static int rb_vec_cursor_y = 0;

static void rb_vec_moveto(int x, int y) {
    rb_vec_cursor_x = x;
    rb_vec_cursor_y = y;
}

static void rb_vec_moveby(int dx, int dy) {
    rb_vec_cursor_x += dx;
    rb_vec_cursor_y += dy;
}

static void rb_vec_lineby(int dx, int dy) {
    rb_display *d = rb_get_current();
    rb_draw_line_internal(rb_vec_cursor_x, rb_vec_cursor_y,
                          rb_vec_cursor_x + dx, rb_vec_cursor_y + dy,
                          d->fg_color, d->fg_brightness);
    rb_vec_moveby(dx, dy);
}

static void rb_vec_drawchar(char ch) {
    const byte *p = rb_vecfont[ch - 0x20];
    byte bright = 0;
    byte x = 0;
    byte y = 0;

    for (byte i = 0; i < 8; i++) {
        byte b = *p++;

        if (b == FONT_LAST) {
            break;
        }
        else if (b == FONT_UP) {
            bright = 0;
        }
        else {
            byte x2 = b >> 4;
            byte y2 = b & 15;

            if (bright == 0) {
                rb_vec_moveby((char)(x2 - x), (char)-(y2 - y));
            }
            else {
                rb_vec_lineby((char)(x2 - x), (char)-(y2 - y));
            }

            bright = 4;
            x = x2;
            y = y2;
        }
    }
}

/* -------------------------------------------------------------------------- */
/*  Public: Character drawing                                                 */
/* -------------------------------------------------------------------------- */

void rb_display_render_draw_char(int x, int y, char ch, int invert, byte paletteColor) {
    rb_display *d = rb_get_current();
    int w = RET_FONT_WIDTH;
    int h = RET_FONT_HEIGHT;
    int offset = (int)ch * 8;
    int stride = d->pixel_width;

    byte *dest = d->pixel_data + y * (stride * 4) + (x * 4);

    ret_color bg = RETPaletteGetColor(d->bg_color);
    ret_color fg = RETPaletteGetColorWithBrightness(paletteColor, d->fg_brightness);

    int xStart = x;
    for (int iy = 0; iy < h; iy++) {
        byte line = ret_font_apple2[offset + iy];

        for (int ix = w - 1; ix >= 0; ix--) {
            if (x >= 0 && x < stride && y >= 0 && y < d->pixel_height) {
                if (ch == ' ' && invert) {
                    *dest = fg.r; dest++;
                    *dest = fg.g; dest++;
                    *dest = fg.b; dest++;
                    *dest = 0xff; dest++;
                }
                else if (rb_getbit(line, ix) == !invert) {
                    *dest = fg.r; dest++;
                    *dest = fg.g; dest++;
                    *dest = fg.b; dest++;
                    *dest = 0xff; dest++;
                }
                else {
                    *dest = bg.r; dest++;
                    *dest = bg.g; dest++;
                    *dest = bg.b; dest++;
                    *dest = 0xff; dest++;
                }
            }

            x++;
        }

        y++;
        x = xStart;

        dest = d->pixel_data + y * (stride * 4) + (x * 4);
    }

    rb_enable_dirty();

    if (d->direct_render) {
        rb_display_render_frame();
    }
}

/* -------------------------------------------------------------------------- */
/*  Public: Screen operations                                                 */
/* -------------------------------------------------------------------------- */

void rb_display_render_clear(void) {
    rb_display *d = rb_get_current();
    int total = d->pixel_width * d->pixel_height;
    int offset = 0;

    ret_color color = RETPaletteGetColor(d->bg_color);

    for (int i = 0; i < total; i++) {
        d->pixel_data[offset] = color.r;
        d->pixel_data[offset + 1] = color.g;
        d->pixel_data[offset + 2] = color.b;
        d->pixel_data[offset + 3] = 255;

        offset += 4;
    }

    rb_enable_dirty();
}

void rb_display_render_scroll_up(int height) {
    rb_display *d = rb_get_current();
    int w = d->pixel_width;
    int h = d->pixel_height;
    byte *source;
    byte *dest;

    for (int y = height; y < h; y++) {
        source = &d->pixel_data[y * w * 4];
        dest = &d->pixel_data[(y - height) * w * 4];
        memcpy(dest, source, w * 4);
    }

    ret_color color = RETPaletteGetColor(d->bg_color);

    int offset = (h - height) * w * 4;
    for (int i = 0; i < height * w; i++) {
        d->pixel_data[offset] = color.r;
        d->pixel_data[offset + 1] = color.g;
        d->pixel_data[offset + 2] = color.b;
        d->pixel_data[offset + 3] = 255;

        offset += 4;
    }

    rb_enable_dirty();
}

void rb_display_render_frame(void) {
    rb_display *d = rb_get_current();

    byte *output = ret_postprocess_get_buffer();
    ret_postprocess_apply(d->pixel_data, output);

    ret_render_frame(output, 0);
}

void rb_display_render_set_direct(int mode) {
    rb_get_current()->direct_render = mode;
}

/* -------------------------------------------------------------------------- */
/*  Public: Color state                                                       */
/* -------------------------------------------------------------------------- */

byte rb_display_set_bg_color(byte index) {
    rb_display *d = rb_get_current();
    byte old = d->bg_color;
    d->bg_color = index;
    return old;
}

byte rb_display_get_bg_color(void) {
    return rb_get_current()->bg_color;
}

byte rb_display_set_fg_color(byte index) {
    rb_display *d = rb_get_current();
    byte old = d->fg_color;
    d->fg_color = index;
    return old;
}

byte rb_display_get_fg_color(void) {
    return rb_get_current()->fg_color;
}

byte rb_display_set_fg_brightness(byte index) {
    rb_display *d = rb_get_current();
    byte old = d->fg_brightness;
    d->fg_brightness = index;
    return old;
}

byte rb_display_get_fg_brightness(void) {
    return rb_get_current()->fg_brightness;
}

void rb_display_set_invert(boolean flag) {
    rb_get_current()->invert_mode = flag;
}

byte rb_display_set_dot_mode(byte mode) {
    rb_display *d = rb_get_current();
    byte old = d->mode_dotted;
    d->mode_dotted = mode;
    return old;
}

/* -------------------------------------------------------------------------- */
/*  Public: Graphics cursor / vector drawing                                  */
/* -------------------------------------------------------------------------- */

void rb_display_move_to(int x, int y) {
    rb_display *d = rb_get_current();
    int hw = d->pixel_width / 2;
    int hh = d->pixel_height / 2;
    d->graphics_x = hw + x;
    d->graphics_y = hh + y;
}

void rb_display_move_by(int x, int y) {
    rb_display *d = rb_get_current();
    d->graphics_x += x;
    d->graphics_y += y;
}

void rb_display_draw_pixel(int x, int y) {
    rb_display *d = rb_get_current();
    int hw = d->pixel_width / 2;
    int hh = d->pixel_height / 2;

    rb_set_pixel(hw + x, hh + y, d->fg_color, d->fg_brightness);

    d->graphics_x = hw + x;
    d->graphics_y = hh + y;
}

void rb_display_draw_line(int x1, int y1, int x2, int y2) {
    rb_display *d = rb_get_current();
    int hw = d->pixel_width / 2;
    int hh = d->pixel_height / 2;

    rb_draw_line_internal(hw + x1, hh + y1, hw + x2, hh + y2,
                          d->fg_color, d->fg_brightness);

    d->graphics_x = hw + x2;
    d->graphics_y = hh + y2;
}

void rb_display_draw_line_to(int x, int y) {
    rb_display *d = rb_get_current();

    rb_draw_line_internal(d->graphics_x, d->graphics_y,
                          d->graphics_x + x, d->graphics_y + y,
                          d->fg_color, d->fg_brightness);

    d->graphics_x += x;
    d->graphics_y += y;
}

void rb_display_draw_vstring(char *str, int x, int y) {
    int x2 = x;

    while (*str != 0) {
        char ch = *str;

        rb_vec_moveto(x2, y + RET_FONT_HEIGHT);
        rb_vec_drawchar(ch);

        str++;
        x2 += RET_FONT_WIDTH + RET_VFONT_SPACE;
    }
}
