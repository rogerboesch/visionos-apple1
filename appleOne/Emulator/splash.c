
#include "splash.h"
#include "rb_display.h"

#include <string.h>

/* Splash timing (in frames at 60 fps) */
#define SPLASH_FADE_IN_FRAMES   90   /* 1.5 seconds */
#define SPLASH_FADE_OUT_FRAMES  45   /* 0.75 seconds */

/* Screen grid: 42 columns x 26 rows (8x8 font, 336x208 pixels) */
#define SPLASH_COLS  42
#define SPLASH_ROWS  26

static int splash_frame_count = 0;
static int splash_active = 0;
static int splash_fading_out = 0;
static int splash_fadeout_frame = 0;

/* Apple logo in ASCII art (16 rows x 14 columns) */
#define LOGO_ROWS 16
#define LOGO_COLS 14

static const char *apple_logo[LOGO_ROWS] = {
    "      ##      ",
    "     ##       ",
    "  ########    ",
    " ##########   ",
    "############  ",
    "############  ",
    "############  ",
    "############  ",
    "############  ",
    " ########### ",
    " ##########   ",
    " ##########   ",
    "  ########    ",
    "  ########    ",
    "   ######     ",
    "    ####      "
};

/* Draw a string centered on a given text row */
static void splash_draw_centered(const char *str, int row) {
    int len = (int)strlen(str);
    int col = (SPLASH_COLS - len) / 2;
    byte color = rb_display_get_fg_color();

    for (int i = 0; i < len; i++) {
        rb_display_text_print_char(row, col + i, str[i], color);
    }
}

/* Draw the apple logo centered, starting at a given text row */
static void splash_draw_logo(int start_row) {
    int col_offset = (SPLASH_COLS - LOGO_COLS) / 2;
    byte color = rb_display_get_fg_color();

    rb_display_set_invert(true);

    for (int row = 0; row < LOGO_ROWS; row++) {
        const char *line = apple_logo[row];

        for (int col = 0; col < LOGO_COLS; col++) {
            if (line[col] == '#') {
                rb_display_text_print_char(start_row + row, col_offset + col, ' ', color);
            }
        }
    }

    rb_display_set_invert(false);
}

void splash_init(void) {
    splash_frame_count = 0;
    splash_active = 1;
    splash_fading_out = 0;
    splash_fadeout_frame = 0;
}

void splash_skip(void) {
    if (!splash_active) return;
    if (splash_fading_out) return;

    splash_fading_out = 1;
    splash_fadeout_frame = 0;
}

int splash_frame(void) {
    if (!splash_active) return 0;

    splash_frame_count++;

    /* Calculate brightness based on phase */
    float alpha;

    if (splash_fading_out) {
        splash_fadeout_frame++;
        alpha = 1.0f - (float)splash_fadeout_frame / (float)SPLASH_FADE_OUT_FRAMES;

        if (splash_fadeout_frame >= SPLASH_FADE_OUT_FRAMES) {
            splash_active = 0;
            rb_display_render_clear();
            rb_display_render_frame();
            return 0;
        }
    }
    else if (splash_frame_count <= SPLASH_FADE_IN_FRAMES) {
        alpha = (float)splash_frame_count / (float)SPLASH_FADE_IN_FRAMES;
    }
    else {
        alpha = 1.0f;
    }

    int brightness = (int)(alpha * 15.0f);
    if (brightness < 0) brightness = 0;
    if (brightness > 15) brightness = 15;

    rb_display_render_clear();

    byte old_fg = rb_display_set_fg_color(RET_COLOR_GREEN);
    byte old_bright = rb_display_set_fg_brightness((unsigned char)brightness);

    rb_display_text_set_immediate(1);

    splash_draw_logo(1);
    splash_draw_centered("50 YEARS OF APPLE", 18);
    splash_draw_centered("APPLE COMPUTER INC", 20);
    splash_draw_centered("1976 - 2026", 22);
    splash_draw_centered("CUPERTINO CALIFORNIA", 24);

    rb_display_text_set_immediate(0);

    rb_display_set_fg_color(old_fg);
    rb_display_set_fg_brightness(old_bright);

    rb_display_render_frame();

    return 1;
}
