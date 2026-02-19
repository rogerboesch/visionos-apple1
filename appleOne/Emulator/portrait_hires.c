#include "portrait_hires.h"
#include "portrait_matrix.h"
#include "portrait_data_jobs.h"
#include "portrait_data_wozniak.h"
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

/* Standard portrait size in text columns/rows.
 * Pixel size = cols*8 x rows*8 = 960x640.
 * Persistent displays avoid use-after-free when Swift reads the buffer async. */
#define PORTRAIT_COLS 100
#define PORTRAIT_ROWS 63

/* Two persistent display slots — created on first use, never destroyed */
static int portrait_display_a = -1;
static int portrait_display_b = -1;

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

int portrait_hires_get_display_a(void) {
    return portrait_display_a;
}

int portrait_hires_get_display_b(void) {
    return portrait_display_b;
}

static int ensure_portrait_display(int *slot) {
    if (*slot < 0) {
        *slot = rb_display_create(PORTRAIT_COLS, PORTRAIT_ROWS);
    }
    else {
        rb_display_text_clear(*slot);
        rb_display_render_clear(*slot);
    }
    return *slot;
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

void portrait_hires_show_jobs(void) {
    int d = ensure_portrait_display(&portrait_display_a);
    render_portrait_to_display(d, portrait_hires_jobs_art,
                                PORTRAIT_HIRES_JOBS_ROWS,
                                PORTRAIT_HIRES_JOBS_COLS);
    rb_render_portrait(rb_display_get_pixel_data(d),
                       rb_display_get_pixel_width(d),
                       rb_display_get_pixel_height(d));
    portrait_matrix_start(portrait_hires_jobs_art,
                          PORTRAIT_HIRES_JOBS_ROWS,
                          PORTRAIT_HIRES_JOBS_COLS);
}

void portrait_hires_show_wozniak(void) {
    int d = ensure_portrait_display(&portrait_display_a);
    render_portrait_to_display(d, portrait_hires_woz_art,
                                PORTRAIT_HIRES_WOZ_ROWS,
                                PORTRAIT_HIRES_WOZ_COLS);
    rb_render_portrait(rb_display_get_pixel_data(d),
                       rb_display_get_pixel_width(d),
                       rb_display_get_pixel_height(d));
    portrait_matrix_start(portrait_hires_woz_art,
                          PORTRAIT_HIRES_WOZ_ROWS,
                          PORTRAIT_HIRES_WOZ_COLS);
}

void portrait_hires_show_both(void) {
    int da = ensure_portrait_display(&portrait_display_a);
    int db = ensure_portrait_display(&portrait_display_b);

    render_portrait_to_display(da, portrait_hires_jobs_art,
                                PORTRAIT_HIRES_JOBS_ROWS,
                                PORTRAIT_HIRES_JOBS_COLS);
    render_portrait_to_display(db, portrait_hires_woz_art,
                                PORTRAIT_HIRES_WOZ_ROWS,
                                PORTRAIT_HIRES_WOZ_COLS);

    rb_render_portrait_pair(rb_display_get_pixel_data(da),
                            rb_display_get_pixel_width(da),
                            rb_display_get_pixel_height(da),
                            rb_display_get_pixel_data(db),
                            rb_display_get_pixel_width(db),
                            rb_display_get_pixel_height(db));
    portrait_matrix_start_pair(portrait_hires_jobs_art,
                               PORTRAIT_HIRES_JOBS_ROWS,
                               PORTRAIT_HIRES_JOBS_COLS,
                               portrait_hires_woz_art,
                               PORTRAIT_HIRES_WOZ_ROWS,
                               PORTRAIT_HIRES_WOZ_COLS);
}
