#include "portrait_hires.h"
#include "portrait_data_jobs.h"
#include "portrait_data_wozniak.h"
#include "ret_platform_types.h"

#include <string.h>

/* Bridge function declared in ObjCBridge.m */
void ret_render_frame_sized(unsigned char *data, int width, int height);

/* Brightness mapping for 4 ASCII density levels + space */
static byte char_to_brightness(char ch) {
    switch (ch) {
        case '#': return 255;
        case '+': return 160;
        case '-': return 90;
        case '.': return 40;
        default:  return 0;   /* space or unknown */
    }
}

/* Phosphor green color at given brightness (R, G, B) */
#define PHOSPHOR_R_FACTOR 3   /* out of 16 */
#define PHOSPHOR_G_FACTOR 16  /* out of 16 */
#define PHOSPHOR_B_FACTOR 2   /* out of 16 */

static void render_portrait(const char **art, int art_rows, int art_cols,
                            byte *buffer, int buf_w, int buf_h) {
    int buf_size = buf_w * buf_h * 4;
    memset(buffer, 0, buf_size);

    for (int row = 0; row < art_rows; row++) {
        const char *line = art[row];
        int len = (int)strlen(line);

        for (int col = 0; col < len && col < art_cols; col++) {
            byte bright = char_to_brightness(line[col]);
            if (bright == 0) {
                continue;
            }

            byte r = (byte)((bright * PHOSPHOR_R_FACTOR) >> 4);
            byte g = (byte)((bright * PHOSPHOR_G_FACTOR) >> 4);
            byte b = (byte)((bright * PHOSPHOR_B_FACTOR) >> 4);

            /* Fill the SCALE x SCALE pixel block */
            for (int sy = 0; sy < PORTRAIT_PIXEL_SCALE; sy++) {
                int py = row * PORTRAIT_PIXEL_SCALE + sy;
                if (py >= buf_h) {
                    break;
                }

                /* Scanline: dim the last row of each block */
                byte sr = r;
                byte sg = g;
                byte sb = b;
                if (sy == PORTRAIT_PIXEL_SCALE - 1) {
                    sr = sr >> 1;
                    sg = sg >> 1;
                    sb = sb >> 1;
                }

                for (int sx = 0; sx < PORTRAIT_PIXEL_SCALE; sx++) {
                    int px = col * PORTRAIT_PIXEL_SCALE + sx;
                    if (px >= buf_w) {
                        break;
                    }

                    int offset = (py * buf_w + px) * 4;
                    buffer[offset + 0] = sr;
                    buffer[offset + 1] = sg;
                    buffer[offset + 2] = sb;
                    buffer[offset + 3] = 255;
                }
            }
        }
    }
}

/* Static buffers — only one is used at a time */
#define JOBS_BUF_W (PORTRAIT_HIRES_JOBS_COLS * PORTRAIT_PIXEL_SCALE)
#define JOBS_BUF_H (PORTRAIT_HIRES_JOBS_ROWS * PORTRAIT_PIXEL_SCALE)
static byte jobs_buffer[JOBS_BUF_W * JOBS_BUF_H * 4];

#define WOZ_BUF_W (PORTRAIT_HIRES_WOZ_COLS * PORTRAIT_PIXEL_SCALE)
#define WOZ_BUF_H (PORTRAIT_HIRES_WOZ_ROWS * PORTRAIT_PIXEL_SCALE)
static byte woz_buffer[WOZ_BUF_W * WOZ_BUF_H * 4];

void portrait_hires_show_jobs(void) {
    render_portrait(portrait_hires_jobs_art,
                    PORTRAIT_HIRES_JOBS_ROWS,
                    PORTRAIT_HIRES_JOBS_COLS,
                    jobs_buffer,
                    JOBS_BUF_W, JOBS_BUF_H);

    ret_render_frame_sized(jobs_buffer, JOBS_BUF_W, JOBS_BUF_H);
}

void portrait_hires_show_wozniak(void) {
    render_portrait(portrait_hires_woz_art,
                    PORTRAIT_HIRES_WOZ_ROWS,
                    PORTRAIT_HIRES_WOZ_COLS,
                    woz_buffer,
                    WOZ_BUF_W, WOZ_BUF_H);

    ret_render_frame_sized(woz_buffer, WOZ_BUF_W, WOZ_BUF_H);
}
