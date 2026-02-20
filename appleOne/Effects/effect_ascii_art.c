#include "effect_ascii_art.h"
#include "effect_art_loader.h"
#include "effect_matrix.h"
#include "rb_display.h"

#include <string.h>

/* Bridge functions declared in ObjCBridge.m */
void rb_render_portrait(unsigned char *data, int width, int height);
void rb_render_portrait_pair(unsigned char *dataA, int widthA, int heightA,
                              unsigned char *dataB, int widthB, int heightB);

/* Phosphor green — must match rb_postprocess.c for consistent color */
#define PHOSPHOR_R 51
#define PHOSPHOR_G 255
#define PHOSPHOR_B 51

/* Per-slot state */
static int        slot_displays[EFFECT_ART_MAX_SLOTS];
static effect_art slot_art[EFFECT_ART_MAX_SLOTS];
static int        slots_initialized = 0;

static void ensure_initialized(void) {
    if (slots_initialized) return;
    for (int i = 0; i < EFFECT_ART_MAX_SLOTS; i++) {
        slot_displays[i] = -1;
        memset(&slot_art[i], 0, sizeof(effect_art));
    }
    slots_initialized = 1;
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

static int ensure_display(int slot, int cols, int rows) {
    if (slot_displays[slot] >= 0) {
        int existing_w = rb_display_get_pixel_width(slot_displays[slot]) / 8;
        int existing_h = rb_display_get_pixel_height(slot_displays[slot]) / 8;
        if (existing_w != cols || existing_h != rows) {
            rb_display_destroy(slot_displays[slot]);
            slot_displays[slot] = rb_display_create(cols, rows);
        }
        else {
            rb_display_text_clear(slot_displays[slot]);
            rb_display_render_clear(slot_displays[slot]);
        }
    }
    else {
        slot_displays[slot] = rb_display_create(cols, rows);
    }
    return slot_displays[slot];
}

static void render_portrait_to_display(int d, const char **art,
                                        int art_rows, int art_cols) {
    rb_display_set_fg_color(d, RB_COLOR_WHITE);
    rb_display_set_fg_brightness(d, 15);
    rb_display_text_set_immediate(d, 1);

    byte color = rb_display_get_fg_color(d);
    for (int row = 0; row < art_rows; row++) {
        const char *line = art[row];
        for (int col = 0; col < art_cols && line[col]; col++) {
            if (line[col] != ' ') {
                rb_display_text_print_char(d, row, col, line[col], color);
            }
        }
    }

    tint_buffer_phosphor(rb_display_get_pixel_data(d),
                         rb_display_get_pixel_width(d),
                         rb_display_get_pixel_height(d));
}

static void push_to_bridge(void) {
    int count = effect_matrix_get_active_count();
    int d0 = effect_ascii_art_get_display(0);

    if (count == 1 && d0 >= 0) {
        rb_render_portrait(rb_display_get_pixel_data(d0),
                           rb_display_get_pixel_width(d0),
                           rb_display_get_pixel_height(d0));
    }
    else if (count >= 2) {
        int d1 = effect_ascii_art_get_display(1);
        if (d0 >= 0 && d1 >= 0) {
            rb_render_portrait_pair(rb_display_get_pixel_data(d0),
                                    rb_display_get_pixel_width(d0),
                                    rb_display_get_pixel_height(d0),
                                    rb_display_get_pixel_data(d1),
                                    rb_display_get_pixel_width(d1),
                                    rb_display_get_pixel_height(d1));
        }
    }
}

/* --- Public API --------------------------------------------------------- */

int effect_ascii_art_show(int slot, const char *name) {
    ensure_initialized();
    if (slot < 0 || slot >= EFFECT_ART_MAX_SLOTS) return -1;

    /* Free old art if any */
    effect_art_free(&slot_art[slot]);

    /* Load new art from file */
    if (!effect_art_load(name, &slot_art[slot])) return -1;

    /* Create/resize display */
    int d = ensure_display(slot, slot_art[slot].cols, slot_art[slot].rows);
    if (d < 0) return -1;

    /* Render art into display */
    render_portrait_to_display(d,
                                (const char **)slot_art[slot].lines,
                                slot_art[slot].rows,
                                slot_art[slot].cols);
    return d;
}

int effect_ascii_art_get_display(int slot) {
    ensure_initialized();
    if (slot < 0 || slot >= EFFECT_ART_MAX_SLOTS) return -1;
    return slot_displays[slot];
}

const char **effect_ascii_art_get_lines(int slot) {
    ensure_initialized();
    if (slot < 0 || slot >= EFFECT_ART_MAX_SLOTS) return NULL;
    return (const char **)slot_art[slot].lines;
}

int effect_ascii_art_get_rows(int slot) {
    ensure_initialized();
    if (slot < 0 || slot >= EFFECT_ART_MAX_SLOTS) return 0;
    return slot_art[slot].rows;
}

int effect_ascii_art_get_cols(int slot) {
    ensure_initialized();
    if (slot < 0 || slot >= EFFECT_ART_MAX_SLOTS) return 0;
    return slot_art[slot].cols;
}

void effect_ascii_art_show_portrait(int slot, const char *name) {
    effect_ascii_art_show(slot, name);
    effect_matrix_start(slot,
                         effect_ascii_art_get_lines(slot),
                         effect_ascii_art_get_rows(slot),
                         effect_ascii_art_get_cols(slot));

    int d = effect_ascii_art_get_display(slot);
    if (d >= 0) {
        rb_render_portrait(rb_display_get_pixel_data(d),
                           rb_display_get_pixel_width(d),
                           rb_display_get_pixel_height(d));
    }
}

void effect_ascii_art_show_portrait_static(int slot, const char *name) {
    effect_ascii_art_show(slot, name);

    int d = effect_ascii_art_get_display(slot);
    if (d >= 0) {
        rb_render_portrait(rb_display_get_pixel_data(d),
                           rb_display_get_pixel_width(d),
                           rb_display_get_pixel_height(d));
    }
}

void effect_ascii_art_trigger_once(int slot) {
    if (slot < 0 || slot >= EFFECT_ART_MAX_SLOTS) return;
    if (slot_displays[slot] < 0) return;
    if (!slot_art[slot].lines) return;

    effect_matrix_start_once(slot,
                              (const char **)slot_art[slot].lines,
                              slot_art[slot].rows,
                              slot_art[slot].cols);
}

void effect_ascii_art_show_portrait_pair(const char *name_a, const char *name_b) {
    effect_ascii_art_show(0, name_a);
    effect_ascii_art_show(1, name_b);
    effect_matrix_start(0,
                         effect_ascii_art_get_lines(0),
                         effect_ascii_art_get_rows(0),
                         effect_ascii_art_get_cols(0));
    effect_matrix_start(1,
                         effect_ascii_art_get_lines(1),
                         effect_ascii_art_get_rows(1),
                         effect_ascii_art_get_cols(1));

    int da = effect_ascii_art_get_display(0);
    int db = effect_ascii_art_get_display(1);
    if (da >= 0 && db >= 0) {
        rb_render_portrait_pair(rb_display_get_pixel_data(da),
                                rb_display_get_pixel_width(da),
                                rb_display_get_pixel_height(da),
                                rb_display_get_pixel_data(db),
                                rb_display_get_pixel_width(db),
                                rb_display_get_pixel_height(db));
    }
}

void effect_ascii_art_frame(void) {
    if (!effect_matrix_is_active()) return;
    effect_matrix_frame();
    push_to_bridge();
}

void effect_ascii_art_stop(void) {
    ensure_initialized();
    effect_matrix_stop();
    for (int i = 0; i < EFFECT_ART_MAX_SLOTS; i++) {
        effect_art_free(&slot_art[i]);
        if (slot_displays[i] >= 0) {
            rb_display_destroy(slot_displays[i]);
            slot_displays[i] = -1;
        }
    }
}
