#include "effect_matrix.h"
#include "effect_hires.h"
#include "rb_display.h"

#include <string.h>

/* Bridge functions declared in ObjCBridge.m */
void rb_render_portrait(unsigned char *data, int width, int height);
void rb_render_portrait_pair(unsigned char *dataA, int widthA, int heightA,
                              unsigned char *dataB, int widthB, int heightB);

/* Phosphor green — must match effect_hires.c */
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
#define PHASE_PAUSE   3
#define PHASE_DONE    4

/* Pause between loops (frames at 60 fps) */
#define MATRIX_PAUSE_FRAMES 300  /* 5 seconds */

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

/* Per-slot animation state */
typedef struct {
    int active;
    int phase;
    int frame_count;
    matrix_column columns[MATRIX_MAX_COLS];
    const char **art;
    int art_rows;
    int art_cols;
    unsigned int seed;
} matrix_slot;

/* Slot A = single or Jobs half, Slot B = Woz half in pair mode */
#define SLOT_A 0
#define SLOT_B 1
#define SLOT_COUNT 2

static matrix_slot slots[SLOT_COUNT];
static int matrix_pair_mode = 0;    /* 1 when both portraits animate together */
static unsigned int invoke_count = 0; /* varies seed across invocations */

/* Simple pseudo-random number generator (per-slot) */
static int slot_rand(matrix_slot *s) {
    s->seed = s->seed * 1103515245 + 12345;
    return (s->seed >> 16) & 0x7FFF;
}

static int slot_rand_range(matrix_slot *s, int min, int max) {
    return min + (slot_rand(s) % (max - min + 1));
}

static char slot_rand_matrix_char(matrix_slot *s) {
    return matrix_chars[slot_rand(s) % MATRIX_CHAR_COUNT];
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

static void init_columns_rain(matrix_slot *s) {
    for (int c = 0; c < s->art_cols; c++) {
        s->columns[c].offset = 0;
        s->columns[c].speed = slot_rand_range(s, MATRIX_MIN_SPEED, MATRIX_MAX_SPEED);
        s->columns[c].delay = slot_rand_range(s, 0, MATRIX_MAX_DELAY);
    }
}

static void init_columns_rebuild(matrix_slot *s) {
    for (int c = 0; c < s->art_cols; c++) {
        s->columns[c].offset = -(s->art_rows + slot_rand_range(s, 5, 30));
        s->columns[c].speed = slot_rand_range(s, MATRIX_MIN_SPEED, MATRIX_MAX_SPEED);
        s->columns[c].delay = slot_rand_range(s, 0, MATRIX_MAX_DELAY);
    }
}

static int all_columns_offscreen(matrix_slot *s) {
    for (int c = 0; c < s->art_cols; c++) {
        if (s->columns[c].offset < s->art_rows + MATRIX_TRAIL_LEN) {
            return 0;
        }
    }
    return 1;
}

static int all_columns_landed(matrix_slot *s) {
    for (int c = 0; c < s->art_cols; c++) {
        if (s->columns[c].offset < 0) {
            return 0;
        }
    }
    return 1;
}

/* Render one slot's rain frame into a display (no bridge call) */
static void render_rain_to_display(matrix_slot *s, int d) {
    rb_display_text_clear(d);
    rb_display_render_clear(d);
    rb_display_set_fg_color(d, RB_COLOR_WHITE);
    rb_display_set_fg_brightness(d, 15);
    rb_display_text_set_immediate(d, 1);
    byte color = rb_display_get_fg_color(d);

    for (int c = 0; c < s->art_cols; c++) {
        int off = s->columns[c].offset;

        for (int r = 0; r < s->art_rows; r++) {
            int screen_row = r + off;
            if (screen_row < 0 || screen_row >= s->art_rows) continue;

            const char *line = s->art[r];
            char ch = (c < (int)strlen(line)) ? line[c] : ' ';
            if (ch != ' ') {
                rb_display_text_print_char(d, screen_row, c, ch, color);
            }
        }

        for (int t = 0; t < MATRIX_TRAIL_LEN; t++) {
            int trail_row = off + s->art_rows + t;
            if (trail_row < 0 || trail_row >= s->art_rows) continue;

            byte trail_brightness = (byte)(15 - t * 4);
            if (trail_brightness < 3) trail_brightness = 3;
            rb_display_set_fg_brightness(d, trail_brightness);
            byte trail_color = rb_display_get_fg_color(d);
            rb_display_text_print_char(d, trail_row, c,
                                       slot_rand_matrix_char(s), trail_color);
        }
        rb_display_set_fg_brightness(d, 15);

        if (s->columns[c].delay > 0) {
            s->columns[c].delay--;
        }
        else {
            s->columns[c].offset += s->columns[c].speed;
        }
    }

    tint_buffer_phosphor(rb_display_get_pixel_data(d),
                         rb_display_get_pixel_width(d),
                         rb_display_get_pixel_height(d));
}

/* Render one slot's rebuild frame into a display (no bridge call) */
static void render_rebuild_to_display(matrix_slot *s, int d) {
    rb_display_text_clear(d);
    rb_display_render_clear(d);
    rb_display_set_fg_color(d, RB_COLOR_WHITE);
    rb_display_set_fg_brightness(d, 15);
    rb_display_text_set_immediate(d, 1);
    byte color = rb_display_get_fg_color(d);

    for (int c = 0; c < s->art_cols; c++) {
        int off = s->columns[c].offset;

        for (int r = 0; r < s->art_rows; r++) {
            int screen_row = r + off;
            if (screen_row < 0 || screen_row >= s->art_rows) continue;

            const char *line = s->art[r];
            char ch = (c < (int)strlen(line)) ? line[c] : ' ';
            if (ch != ' ') {
                rb_display_text_print_char(d, screen_row, c, ch, color);
            }
        }

        for (int t = 0; t < MATRIX_TRAIL_LEN; t++) {
            int trail_row = off - 1 - t;
            if (trail_row < 0 || trail_row >= s->art_rows) continue;

            byte trail_brightness = (byte)(15 - t * 4);
            if (trail_brightness < 3) trail_brightness = 3;
            rb_display_set_fg_brightness(d, trail_brightness);
            byte trail_color = rb_display_get_fg_color(d);
            rb_display_text_print_char(d, trail_row, c,
                                       slot_rand_matrix_char(s), trail_color);
        }
        rb_display_set_fg_brightness(d, 15);

        if (s->columns[c].delay > 0) {
            s->columns[c].delay--;
        }
        else {
            s->columns[c].offset += s->columns[c].speed;
            if (s->columns[c].offset > 0) {
                s->columns[c].offset = 0;
            }
        }
    }

    tint_buffer_phosphor(rb_display_get_pixel_data(d),
                         rb_display_get_pixel_width(d),
                         rb_display_get_pixel_height(d));
}

/* Render final clean portrait into a display (no bridge call) */
static void render_final_to_display(matrix_slot *s, int d) {
    rb_display_text_clear(d);
    rb_display_render_clear(d);
    rb_display_set_fg_color(d, RB_COLOR_WHITE);
    rb_display_set_fg_brightness(d, 15);
    rb_display_text_set_immediate(d, 1);
    byte color = rb_display_get_fg_color(d);

    for (int r = 0; r < s->art_rows; r++) {
        const char *line = s->art[r];
        for (int c = 0; c < s->art_cols && line[c]; c++) {
            if (line[c] != ' ') {
                rb_display_text_print_char(d, r, c, line[c], color);
            }
        }
    }

    tint_buffer_phosphor(rb_display_get_pixel_data(d),
                         rb_display_get_pixel_width(d),
                         rb_display_get_pixel_height(d));
}

/* Restart a slot for the next loop iteration with a new seed */
static void restart_slot(matrix_slot *s) {
    unsigned int new_seed = s->seed * 1103515245 + 12345;
    const char **art = s->art;
    int rows = s->art_rows;
    int cols = s->art_cols;
    memset(s, 0, sizeof(matrix_slot));
    s->art = art;
    s->art_rows = rows;
    s->art_cols = cols;
    s->phase = PHASE_HOLD;
    s->frame_count = 0;
    s->active = 1;
    s->seed = new_seed;
}

/* Advance one slot's state machine. Returns 1 if still animating. */
static int advance_slot(matrix_slot *s, int d) {
    if (!s->active) return 0;

    s->frame_count++;

    switch (s->phase) {
        case PHASE_HOLD:
            if (s->frame_count >= MATRIX_HOLD_FRAMES) {
                s->phase = PHASE_RAIN;
                s->frame_count = 0;
                init_columns_rain(s);
            }
            break;

        case PHASE_RAIN:
            render_rain_to_display(s, d);
            if (all_columns_offscreen(s)) {
                s->phase = PHASE_REBUILD;
                s->frame_count = 0;
                init_columns_rebuild(s);
            }
            break;

        case PHASE_REBUILD:
            render_rebuild_to_display(s, d);
            if (all_columns_landed(s)) {
                render_final_to_display(s, d);
                s->phase = PHASE_PAUSE;
                s->frame_count = 0;
            }
            break;

        case PHASE_PAUSE:
            /* Show final portrait, then restart the cycle */
            if (s->frame_count >= MATRIX_PAUSE_FRAMES) {
                restart_slot(s);
            }
            break;

        default:
            s->active = 0;
            return 0;
    }

    return 1;
}

static void init_slot(matrix_slot *s, const char **art, int rows, int cols,
                      unsigned int seed) {
    memset(s, 0, sizeof(matrix_slot));
    s->art = art;
    s->art_rows = rows;
    s->art_cols = cols;
    s->phase = PHASE_HOLD;
    s->frame_count = 0;
    s->active = 1;
    s->seed = seed;
}

/* --- Public API --------------------------------------------------------- */

void effect_matrix_start(const char **art, int rows, int cols) {
    /* Cancel any previous pair mode */
    slots[SLOT_B].active = 0;
    matrix_pair_mode = 0;
    invoke_count++;

    init_slot(&slots[SLOT_A], art, rows, cols,
              (unsigned int)(rows * 7919 + cols * 104729 + invoke_count * 31337));
}

void effect_matrix_start_pair(const char **art_a, int rows_a, int cols_a,
                                const char **art_b, int rows_b, int cols_b) {
    matrix_pair_mode = 1;
    invoke_count++;

    init_slot(&slots[SLOT_A], art_a, rows_a, cols_a,
              (unsigned int)(rows_a * 7919 + cols_a * 104729 + invoke_count * 31337));
    init_slot(&slots[SLOT_B], art_b, rows_b, cols_b,
              (unsigned int)(rows_b * 6311 + cols_b * 87491 + invoke_count * 48611));
}

int effect_matrix_frame(void) {
    if (!slots[SLOT_A].active && !slots[SLOT_B].active) return 0;

    int da = effect_hires_get_display_a();
    if (da < 0) {
        slots[SLOT_A].active = 0;
        slots[SLOT_B].active = 0;
        return 0;
    }

    if (matrix_pair_mode) {
        int db = effect_hires_get_display_b();
        if (db < 0) {
            slots[SLOT_A].active = 0;
            slots[SLOT_B].active = 0;
            return 0;
        }

        /* During hold/pause, portrait is static — skip bridge call */
        int a_static = (slots[SLOT_A].phase == PHASE_HOLD || slots[SLOT_A].phase == PHASE_PAUSE);
        int b_static = (slots[SLOT_B].phase == PHASE_HOLD || slots[SLOT_B].phase == PHASE_PAUSE);
        if (a_static && b_static) {
            advance_slot(&slots[SLOT_A], da);
            advance_slot(&slots[SLOT_B], db);
            return 1;
        }

        advance_slot(&slots[SLOT_A], da);
        advance_slot(&slots[SLOT_B], db);

        rb_render_portrait_pair(rb_display_get_pixel_data(da),
                                rb_display_get_pixel_width(da),
                                rb_display_get_pixel_height(da),
                                rb_display_get_pixel_data(db),
                                rb_display_get_pixel_width(db),
                                rb_display_get_pixel_height(db));

        return (slots[SLOT_A].active || slots[SLOT_B].active) ? 1 : 0;
    }
    else {
        int still = advance_slot(&slots[SLOT_A], da);
        /* During hold/pause, portrait is static — skip bridge call */
        if (slots[SLOT_A].phase != PHASE_HOLD && slots[SLOT_A].phase != PHASE_PAUSE) {
            rb_render_portrait(rb_display_get_pixel_data(da),
                               rb_display_get_pixel_width(da),
                               rb_display_get_pixel_height(da));
        }
        return still;
    }
}

void effect_matrix_stop(void) {
    slots[SLOT_A].active = 0;
    slots[SLOT_B].active = 0;
    matrix_pair_mode = 0;
}

int effect_matrix_is_active(void) {
    return slots[SLOT_A].active || slots[SLOT_B].active;
}
