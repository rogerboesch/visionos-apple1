#include "portrait_matrix.h"
#include "portrait_hires.h"
#include "rb_display.h"

#include <string.h>

/* Bridge function declared in ObjCBridge.m */
void rb_render_portrait(unsigned char *data, int width, int height);

/* Phosphor green — must match portrait_hires.c */
#define PHOSPHOR_R 51
#define PHOSPHOR_G 255
#define PHOSPHOR_B 51

/* Animation timing (frames at 60 fps) */
#define MATRIX_HOLD_FRAMES    60   /* 1 second */
#define MATRIX_RAIN_FRAMES    90   /* 1.5 seconds max */
#define MATRIX_REBUILD_FRAMES 90   /* 1.5 seconds max */

/* Column animation parameters */
#define MATRIX_MIN_SPEED  1
#define MATRIX_MAX_SPEED  3
#define MATRIX_MAX_DELAY  15
#define MATRIX_TRAIL_LEN  3

/* Phases */
#define PHASE_HOLD    0
#define PHASE_RAIN    1
#define PHASE_REBUILD 2
#define PHASE_DONE    3

/* Max portrait dimensions */
#define MATRIX_MAX_COLS 100
#define MATRIX_MAX_ROWS 63

/* Random chars used for the matrix trail effect */
static const char matrix_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
    "0123456789@#$%&*+=<>{}[]|/\\~^";

#define MATRIX_CHAR_COUNT (sizeof(matrix_chars) - 1)

typedef struct {
    int offset;   /* vertical shift in rows (positive=down, negative=up) */
    int speed;    /* rows to shift per frame (1-3) */
    int delay;    /* frames to wait before column starts moving */
} matrix_column;

static int matrix_active = 0;
static int matrix_phase = PHASE_DONE;
static int matrix_frame_count = 0;
static matrix_column columns[MATRIX_MAX_COLS];

/* Cached art pointer and dimensions */
static const char **matrix_art = NULL;
static int matrix_art_rows = 0;
static int matrix_art_cols = 0;

/* Simple pseudo-random number generator */
static unsigned int matrix_seed = 12345;

static int matrix_rand(void) {
    matrix_seed = matrix_seed * 1103515245 + 12345;
    return (matrix_seed >> 16) & 0x7FFF;
}

static int rand_range(int min, int max) {
    return min + (matrix_rand() % (max - min + 1));
}

static char rand_matrix_char(void) {
    return matrix_chars[matrix_rand() % MATRIX_CHAR_COUNT];
}

static void tint_buffer_phosphor(byte *buffer, int buf_w, int buf_h) {
    int total = buf_w * buf_h;
    for (int i = 0; i < total; i++) {
        int off = i * 4;
        int brightness = buffer[off];
        if (buffer[off + 1] > brightness) brightness = buffer[off + 1];
        if (buffer[off + 2] > brightness) brightness = buffer[off + 2];

        if (brightness > 0) {
            float t = (float)brightness / 255.0f;
            buffer[off]     = (byte)(PHOSPHOR_R * t);
            buffer[off + 1] = (byte)(PHOSPHOR_G * t);
            buffer[off + 2] = (byte)(PHOSPHOR_B * t);
        }
    }
}

static void init_columns_rain(void) {
    for (int c = 0; c < matrix_art_cols; c++) {
        columns[c].offset = 0;
        columns[c].speed = rand_range(MATRIX_MIN_SPEED, MATRIX_MAX_SPEED);
        columns[c].delay = rand_range(0, MATRIX_MAX_DELAY);
    }
}

static void init_columns_rebuild(void) {
    for (int c = 0; c < matrix_art_cols; c++) {
        /* Start above screen — negative offset means chars are above */
        columns[c].offset = -(matrix_art_rows + rand_range(5, 30));
        columns[c].speed = rand_range(MATRIX_MIN_SPEED, MATRIX_MAX_SPEED);
        columns[c].delay = rand_range(0, MATRIX_MAX_DELAY);
    }
}

static int all_columns_offscreen(void) {
    for (int c = 0; c < matrix_art_cols; c++) {
        if (columns[c].offset < matrix_art_rows + MATRIX_TRAIL_LEN) {
            return 0;
        }
    }
    return 1;
}

static int all_columns_landed(void) {
    for (int c = 0; c < matrix_art_cols; c++) {
        if (columns[c].offset < 0) {
            return 0;
        }
    }
    return 1;
}

static void render_rain_frame(int d) {
    rb_display_render_clear(d);
    rb_display_text_clear(d);
    rb_display_set_fg_color(d, RB_COLOR_WHITE);
    rb_display_set_fg_brightness(d, 15);
    rb_display_text_set_immediate(d, 1);
    byte color = rb_display_get_fg_color(d);

    for (int c = 0; c < matrix_art_cols; c++) {
        int off = columns[c].offset;

        /* Draw original art chars shifted down by offset */
        for (int r = 0; r < matrix_art_rows; r++) {
            int screen_row = r + off;
            if (screen_row < 0 || screen_row >= matrix_art_rows) continue;

            const char *line = matrix_art[r];
            char ch = (c < (int)strlen(line)) ? line[c] : ' ';
            if (ch != ' ') {
                rb_display_text_print_char(d, screen_row, c, ch, color);
            }
        }

        /* Draw matrix trail at leading edge (just below the shifted art) */
        for (int t = 0; t < MATRIX_TRAIL_LEN; t++) {
            int trail_row = off + matrix_art_rows + t;
            if (trail_row < 0 || trail_row >= matrix_art_rows) continue;

            /* Brighter at the leading edge, dimmer further back */
            byte trail_brightness = (byte)(15 - t * 4);
            if (trail_brightness < 3) trail_brightness = 3;
            rb_display_set_fg_brightness(d, trail_brightness);
            byte trail_color = rb_display_get_fg_color(d);
            rb_display_text_print_char(d, trail_row, c, rand_matrix_char(),
                                       trail_color);
        }
        rb_display_set_fg_brightness(d, 15);

        /* Advance column */
        if (columns[c].delay > 0) {
            columns[c].delay--;
        }
        else {
            columns[c].offset += columns[c].speed;
        }
    }

    tint_buffer_phosphor(rb_display_get_pixel_data(d),
                         rb_display_get_pixel_width(d),
                         rb_display_get_pixel_height(d));
    rb_render_portrait(rb_display_get_pixel_data(d),
                       rb_display_get_pixel_width(d),
                       rb_display_get_pixel_height(d));
}

static void render_rebuild_frame(int d) {
    rb_display_render_clear(d);
    rb_display_text_clear(d);
    rb_display_set_fg_color(d, RB_COLOR_WHITE);
    rb_display_set_fg_brightness(d, 15);
    rb_display_text_set_immediate(d, 1);
    byte color = rb_display_get_fg_color(d);

    for (int c = 0; c < matrix_art_cols; c++) {
        int off = columns[c].offset;

        /* Draw original art chars shifted by offset (negative = above) */
        for (int r = 0; r < matrix_art_rows; r++) {
            int screen_row = r + off;
            if (screen_row < 0 || screen_row >= matrix_art_rows) continue;

            const char *line = matrix_art[r];
            char ch = (c < (int)strlen(line)) ? line[c] : ' ';
            if (ch != ' ') {
                rb_display_text_print_char(d, screen_row, c, ch, color);
            }
        }

        /* Draw matrix trail above the falling art (leading edge) */
        for (int t = 0; t < MATRIX_TRAIL_LEN; t++) {
            int trail_row = off - 1 - t;
            if (trail_row < 0 || trail_row >= matrix_art_rows) continue;

            byte trail_brightness = (byte)(15 - t * 4);
            if (trail_brightness < 3) trail_brightness = 3;
            rb_display_set_fg_brightness(d, trail_brightness);
            byte trail_color = rb_display_get_fg_color(d);
            rb_display_text_print_char(d, trail_row, c, rand_matrix_char(),
                                       trail_color);
        }
        rb_display_set_fg_brightness(d, 15);

        /* Advance column toward final position (offset 0) */
        if (columns[c].delay > 0) {
            columns[c].delay--;
        }
        else {
            columns[c].offset += columns[c].speed;
            if (columns[c].offset > 0) {
                columns[c].offset = 0;  /* snap to final position */
            }
        }
    }

    tint_buffer_phosphor(rb_display_get_pixel_data(d),
                         rb_display_get_pixel_width(d),
                         rb_display_get_pixel_height(d));
    rb_render_portrait(rb_display_get_pixel_data(d),
                       rb_display_get_pixel_width(d),
                       rb_display_get_pixel_height(d));
}

static void render_final_portrait(int d) {
    rb_display_render_clear(d);
    rb_display_text_clear(d);
    rb_display_set_fg_color(d, RB_COLOR_WHITE);
    rb_display_set_fg_brightness(d, 15);
    rb_display_text_set_immediate(d, 1);
    byte color = rb_display_get_fg_color(d);

    for (int r = 0; r < matrix_art_rows; r++) {
        const char *line = matrix_art[r];
        for (int c = 0; c < matrix_art_cols && line[c]; c++) {
            if (line[c] != ' ') {
                rb_display_text_print_char(d, r, c, line[c], color);
            }
        }
    }

    tint_buffer_phosphor(rb_display_get_pixel_data(d),
                         rb_display_get_pixel_width(d),
                         rb_display_get_pixel_height(d));
    rb_render_portrait(rb_display_get_pixel_data(d),
                       rb_display_get_pixel_width(d),
                       rb_display_get_pixel_height(d));
}

/* --- Public API --------------------------------------------------------- */

void portrait_matrix_start(const char **art, int rows, int cols) {
    matrix_art = art;
    matrix_art_rows = rows;
    matrix_art_cols = cols;
    matrix_phase = PHASE_HOLD;
    matrix_frame_count = 0;
    matrix_active = 1;
    /* Reseed RNG for variety each time */
    matrix_seed = (unsigned int)(rows * 7919 + cols * 104729);
}

int portrait_matrix_frame(void) {
    if (!matrix_active) return 0;

    matrix_frame_count++;
    int d = portrait_hires_get_display_a();
    if (d < 0) {
        matrix_active = 0;
        return 0;
    }

    switch (matrix_phase) {
        case PHASE_HOLD:
            if (matrix_frame_count >= MATRIX_HOLD_FRAMES) {
                matrix_phase = PHASE_RAIN;
                matrix_frame_count = 0;
                init_columns_rain();
            }
            break;

        case PHASE_RAIN:
            render_rain_frame(d);
            if (all_columns_offscreen()) {
                matrix_phase = PHASE_REBUILD;
                matrix_frame_count = 0;
                init_columns_rebuild();
            }
            break;

        case PHASE_REBUILD:
            render_rebuild_frame(d);
            if (all_columns_landed()) {
                render_final_portrait(d);
                matrix_phase = PHASE_DONE;
                matrix_active = 0;
                return 0;
            }
            break;

        default:
            matrix_active = 0;
            return 0;
    }

    return 1;
}

int portrait_matrix_is_active(void) {
    return matrix_active;
}
