#include "portrait_hires.h"
#include "portrait_data_jobs.h"
#include "portrait_data_wozniak.h"
#include "ret_platform_types.h"
#include "ret_renderer.h"
#include "ret_textbuffer.h"
#include "ret_font.h"

#include <string.h>

/* Bridge function declared in ObjCBridge.m */
void ret_render_frame_sized(unsigned char *data, int width, int height);

/* Buffer dimensions: cols * font_width, rows * font_height */
#define JOBS_BUF_W (PORTRAIT_HIRES_JOBS_COLS * RET_FONT_WIDTH)
#define JOBS_BUF_H (PORTRAIT_HIRES_JOBS_ROWS * RET_FONT_HEIGHT)
static byte jobs_buffer[JOBS_BUF_W * JOBS_BUF_H * 4];

#define WOZ_BUF_W (PORTRAIT_HIRES_WOZ_COLS * RET_FONT_WIDTH)
#define WOZ_BUF_H (PORTRAIT_HIRES_WOZ_ROWS * RET_FONT_HEIGHT)
static byte woz_buffer[WOZ_BUF_W * WOZ_BUF_H * 4];

static void render_portrait_glyphs(const char **art, int art_rows, int art_cols,
                                   byte *buffer, int buf_w, int buf_h) {
    memset(buffer, 0, buf_w * buf_h * 4);

    /* Resize text buffer to match portrait grid */
    ret_text_resize(art_cols, art_rows);

    /* Set custom render target */
    ret_rend_set_custom_target(buffer, buf_w, buf_h);

    /* Set phosphor green color */
    byte old_fg = RETSetFgColor(RET_COLOR_GREEN);
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

    /* Send to display */
    ret_render_frame_sized(buffer, buf_w, buf_h);
}

void portrait_hires_show_jobs(void) {
    render_portrait_glyphs(portrait_hires_jobs_art,
                           PORTRAIT_HIRES_JOBS_ROWS,
                           PORTRAIT_HIRES_JOBS_COLS,
                           jobs_buffer,
                           JOBS_BUF_W, JOBS_BUF_H);
}

void portrait_hires_show_wozniak(void) {
    render_portrait_glyphs(portrait_hires_woz_art,
                           PORTRAIT_HIRES_WOZ_ROWS,
                           PORTRAIT_HIRES_WOZ_COLS,
                           woz_buffer,
                           WOZ_BUF_W, WOZ_BUF_H);
}
