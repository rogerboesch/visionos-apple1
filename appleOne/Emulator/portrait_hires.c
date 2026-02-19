#include "portrait_hires.h"
#include "portrait_data_jobs.h"
#include "portrait_data_wozniak.h"
#include "ret_platform_types.h"
#include "ret_renderer.h"
#include "ret_textbuffer.h"
#include "ret_font.h"

#include <string.h>

/* Bridge functions declared in ObjCBridge.m */
void ret_render_portrait(unsigned char *data, int width, int height);
void ret_render_portrait_pair(unsigned char *dataA, int widthA, int heightA,
                              unsigned char *dataB, int widthB, int heightB);

/* Phosphor green — must match ret_postprocess.c for consistent color */
#define PHOSPHOR_R 51
#define PHOSPHOR_G 255
#define PHOSPHOR_B 51

/* Buffer dimensions: cols * font_width, rows * font_height */
#define JOBS_BUF_W (PORTRAIT_HIRES_JOBS_COLS * RET_FONT_WIDTH)
#define JOBS_BUF_H (PORTRAIT_HIRES_JOBS_ROWS * RET_FONT_HEIGHT)
static byte jobs_buffer[JOBS_BUF_W * JOBS_BUF_H * 4];

#define WOZ_BUF_W (PORTRAIT_HIRES_WOZ_COLS * RET_FONT_WIDTH)
#define WOZ_BUF_H (PORTRAIT_HIRES_WOZ_ROWS * RET_FONT_HEIGHT)
static byte woz_buffer[WOZ_BUF_W * WOZ_BUF_H * 4];

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

static void render_portrait_to_buffer(const char **art, int art_rows, int art_cols,
                                      byte *buffer, int buf_w, int buf_h) {
    memset(buffer, 0, buf_w * buf_h * 4);

    /* Resize text buffer to match portrait grid */
    ret_text_resize(art_cols, art_rows);

    /* Set custom render target */
    ret_rend_set_custom_target(buffer, buf_w, buf_h);

    /* Render glyphs as white — we tint to phosphor green afterwards */
    byte old_fg = RETSetFgColor(RET_COLOR_WHITE);
    byte old_bright = RETSetFgBrightness(15);

    /* Draw each character using the bitmap font */
    for (int row = 0; row < art_rows; row++) {
        const char *line = art[row];
        for (int col = 0; col < art_cols && line[col]; col++) {
            char ch = line[col];
            if (ch == ' ') {
                continue;
            }
            int px = col * RET_FONT_WIDTH;
            int py = row * RET_FONT_HEIGHT;
            ret_rend_draw_char(px, py, ch, 0, RETGetFgColor());
        }
    }

    /* Restore renderer and text buffer */
    ret_rend_restore_target();
    ret_text_restore_default();

    RETSetFgColor(old_fg);
    RETSetFgBrightness(old_bright);

    /* Tint white glyphs to phosphor green (matches post-processor) */
    tint_buffer_phosphor(buffer, buf_w, buf_h);
}

void portrait_hires_show_jobs(void) {
    render_portrait_to_buffer(portrait_hires_jobs_art,
                              PORTRAIT_HIRES_JOBS_ROWS,
                              PORTRAIT_HIRES_JOBS_COLS,
                              jobs_buffer,
                              JOBS_BUF_W, JOBS_BUF_H);
    ret_render_portrait(jobs_buffer, JOBS_BUF_W, JOBS_BUF_H);
}

void portrait_hires_show_wozniak(void) {
    render_portrait_to_buffer(portrait_hires_woz_art,
                              PORTRAIT_HIRES_WOZ_ROWS,
                              PORTRAIT_HIRES_WOZ_COLS,
                              woz_buffer,
                              WOZ_BUF_W, WOZ_BUF_H);
    ret_render_portrait(woz_buffer, WOZ_BUF_W, WOZ_BUF_H);
}

void portrait_hires_show_both(void) {
    render_portrait_to_buffer(portrait_hires_jobs_art,
                              PORTRAIT_HIRES_JOBS_ROWS,
                              PORTRAIT_HIRES_JOBS_COLS,
                              jobs_buffer,
                              JOBS_BUF_W, JOBS_BUF_H);

    render_portrait_to_buffer(portrait_hires_woz_art,
                              PORTRAIT_HIRES_WOZ_ROWS,
                              PORTRAIT_HIRES_WOZ_COLS,
                              woz_buffer,
                              WOZ_BUF_W, WOZ_BUF_H);

    ret_render_portrait_pair(jobs_buffer, JOBS_BUF_W, JOBS_BUF_H,
                             woz_buffer, WOZ_BUF_W, WOZ_BUF_H);
}
