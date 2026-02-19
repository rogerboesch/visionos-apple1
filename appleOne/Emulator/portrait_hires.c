#include "portrait_hires.h"
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

static int render_portrait_to_buffer(const char **art, int art_rows, int art_cols,
                                     byte **out_pixels, int *out_w, int *out_h) {
    /* Create a temporary display sized for this portrait */
    int d = rb_display_create(art_cols, art_rows);

    /* Render glyphs as white — we tint to phosphor green afterwards */
    rb_display_set_fg_color(d, RB_COLOR_WHITE);
    rb_display_set_fg_brightness(d, 15);
    rb_display_text_set_immediate(d, 1);

    /* Draw each character using the bitmap font */
    byte color = rb_display_get_fg_color(d);
    for (int row = 0; row < art_rows; row++) {
        const char *line = art[row];
        for (int col = 0; col < art_cols && line[col]; col++) {
            char ch = line[col];
            if (ch == ' ') {
                continue;
            }
            rb_display_text_print_char(d, row, col, ch, color);
        }
    }

    rb_display_text_set_immediate(d, 0);

    int w = rb_display_get_pixel_width(d);
    int h = rb_display_get_pixel_height(d);
    byte *pixels = rb_display_get_pixel_data(d);

    /* Tint white glyphs to phosphor green (matches post-processor) */
    tint_buffer_phosphor(pixels, w, h);

    *out_pixels = pixels;
    *out_w = w;
    *out_h = h;

    return d;
}

void portrait_hires_show_jobs(void) {
    byte *pixels;
    int w, h;
    int d = render_portrait_to_buffer(portrait_hires_jobs_art,
                                      PORTRAIT_HIRES_JOBS_ROWS,
                                      PORTRAIT_HIRES_JOBS_COLS,
                                      &pixels, &w, &h);
    rb_render_portrait(pixels, w, h);
    rb_display_destroy(d);
}

void portrait_hires_show_wozniak(void) {
    byte *pixels;
    int w, h;
    int d = render_portrait_to_buffer(portrait_hires_woz_art,
                                      PORTRAIT_HIRES_WOZ_ROWS,
                                      PORTRAIT_HIRES_WOZ_COLS,
                                      &pixels, &w, &h);
    rb_render_portrait(pixels, w, h);
    rb_display_destroy(d);
}

void portrait_hires_show_both(void) {
    byte *jobs_pixels, *woz_pixels;
    int jobs_w, jobs_h, woz_w, woz_h;

    int d_jobs = render_portrait_to_buffer(portrait_hires_jobs_art,
                                           PORTRAIT_HIRES_JOBS_ROWS,
                                           PORTRAIT_HIRES_JOBS_COLS,
                                           &jobs_pixels, &jobs_w, &jobs_h);

    int d_woz = render_portrait_to_buffer(portrait_hires_woz_art,
                                          PORTRAIT_HIRES_WOZ_ROWS,
                                          PORTRAIT_HIRES_WOZ_COLS,
                                          &woz_pixels, &woz_w, &woz_h);

    rb_render_portrait_pair(jobs_pixels, jobs_w, jobs_h,
                             woz_pixels, woz_w, woz_h);

    rb_display_destroy(d_jobs);
    rb_display_destroy(d_woz);
}
