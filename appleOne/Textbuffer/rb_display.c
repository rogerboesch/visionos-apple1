
#include "rb_display.h"

#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/*  Display array and current display tracking                                */
/* -------------------------------------------------------------------------- */

static rb_display rb_displays[RB_DISPLAY_MAX];
static int rb_current_display = 0;

rb_display *rb_get_current(void) {
    return &rb_displays[rb_current_display];
}

rb_display *rb_get_display(int index) {
    if (index < 0 || index >= RB_DISPLAY_MAX) {
        return &rb_displays[0];
    }
    return &rb_displays[index];
}

/* -------------------------------------------------------------------------- */
/*  Internal: allocate buffers for a display                                  */
/* -------------------------------------------------------------------------- */

static void rb_display_alloc_buffers(rb_display *d, int cols, int rows) {
    int text_size = cols * rows;
    int pixel_w = cols * RB_FONT_WIDTH;
    int pixel_h = rows * RB_FONT_HEIGHT;
    int pixel_size = pixel_w * pixel_h * 4;

    d->text_cols = cols;
    d->text_rows = rows;
    d->pixel_width = pixel_w;
    d->pixel_height = pixel_h;

    d->text_buffer = (byte *)malloc(text_size);
    d->text_colbuf = (byte *)malloc(text_size);
    d->line_buffer = (char *)malloc(cols);
    d->pixel_data = (byte *)malloc(pixel_size);

    memset(d->text_buffer, 0, text_size);
    memset(d->text_colbuf, 0, text_size);
    memset(d->line_buffer, 0, cols);
    memset(d->pixel_data, 0, pixel_size);
}

static void rb_display_init_defaults(rb_display *d) {
    d->cursor_row = 0;
    d->cursor_col = 0;
    d->cursor_blink = 0;
    d->cursor_cycles = 0;
    d->cursor_visible = true;

    d->bg_color = RB_COLOR_BG;
    d->fg_color = RB_COLOR_FG;
    d->fg_brightness = RB_BRIGHTNESS_NORMAL;
    d->invert_mode = false;

    d->graphics_x = 0;
    d->graphics_y = 0;
    d->mode_dotted = 0;

    d->dirty = 0;
    d->immediate_print = 1;
    d->direct_render = 0;
}

/* -------------------------------------------------------------------------- */
/*  Lifecycle                                                                 */
/* -------------------------------------------------------------------------- */

void rb_display_init(int cols, int rows) {
    memset(rb_displays, 0, sizeof(rb_displays));

    /* Create display 0 — the main terminal */
    rb_displays[0].index = 0;
    rb_displays[0].active = 1;
    rb_display_alloc_buffers(&rb_displays[0], cols, rows);
    rb_display_init_defaults(&rb_displays[0]);

    rb_current_display = 0;
}

int rb_display_create(int cols, int rows) {
    for (int i = 1; i < RB_DISPLAY_MAX; i++) {
        if (!rb_displays[i].active) {
            rb_displays[i].index = i;
            rb_displays[i].active = 1;
            rb_display_alloc_buffers(&rb_displays[i], cols, rows);
            rb_display_init_defaults(&rb_displays[i]);
            return i;
        }
    }
    return -1;
}

void rb_display_destroy(int display) {
    if (display <= 0 || display >= RB_DISPLAY_MAX) {
        return;
    }

    rb_display *d = &rb_displays[display];
    if (!d->active) {
        return;
    }

    free(d->text_buffer);
    free(d->text_colbuf);
    free(d->line_buffer);
    free(d->pixel_data);

    memset(d, 0, sizeof(rb_display));
}

void rb_display_set_current(int display) {
    if (display >= 0 && display < RB_DISPLAY_MAX && rb_displays[display].active) {
        rb_current_display = display;
    }
}

int rb_display_get_current(void) {
    return rb_current_display;
}

/* -------------------------------------------------------------------------- */
/*  Dimension getters                                                         */
/* -------------------------------------------------------------------------- */

int rb_display_get_text_width(int display) {
    return rb_get_display(display)->text_cols;
}

int rb_display_get_text_height(int display) {
    return rb_get_display(display)->text_rows;
}

int rb_display_get_pixel_width(int display) {
    return rb_get_display(display)->pixel_width;
}

int rb_display_get_pixel_height(int display) {
    return rb_get_display(display)->pixel_height;
}

byte *rb_display_get_pixel_data(int display) {
    return rb_get_display(display)->pixel_data;
}
