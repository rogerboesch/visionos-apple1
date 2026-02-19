
#include "splash.h"
#include "ret_renderer.h"
#include "ret_textbuffer.h"

#include <string.h>

// Splash timing (in frames at 60 fps)
#define SPLASH_FADE_IN_FRAMES   90   // 1.5 seconds
#define SPLASH_HOLD_FRAMES      120  // 2.0 seconds
#define SPLASH_FADE_OUT_FRAMES  45   // 0.75 seconds
#define SPLASH_TOTAL_FRAMES     (SPLASH_FADE_IN_FRAMES + SPLASH_HOLD_FRAMES + SPLASH_FADE_OUT_FRAMES)

// Screen grid: 42 columns x 26 rows (8x8 font, 336x208 pixels)
#define SPLASH_COLS  42
#define SPLASH_ROWS  26

static int splash_frame_count = 0;
static int splash_active = 0;

// Apple logo in ASCII art (16 rows x 14 columns)
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

// Draw a string centered on a given text row
static void splash_draw_centered(const char *str, int row) {
    int len = (int)strlen(str);
    int col = (SPLASH_COLS - len) / 2;

    for (int i = 0; i < len; i++) {
        int px = (col + i) * RET_FONT_WIDTH;
        int py = row * RET_FONT_HEIGHT;
        ret_rend_draw_char(px, py, str[i], 0, RETGetFgColor());
    }
}

// Draw the apple logo centered, starting at a given text row
static void splash_draw_logo(int start_row) {
    int col_offset = (SPLASH_COLS - LOGO_COLS) / 2;

    for (int row = 0; row < LOGO_ROWS; row++) {
        const char *line = apple_logo[row];

        for (int col = 0; col < LOGO_COLS; col++) {
            if (line[col] == '#') {
                int px = (col_offset + col) * RET_FONT_WIDTH;
                int py = (start_row + row) * RET_FONT_HEIGHT;
                // Draw a solid block character (inverted space)
                ret_rend_draw_char(px, py, ' ', 1, RETGetFgColor());
            }
        }
    }
}

void splash_init(void) {
    splash_frame_count = 0;
    splash_active = 1;
}

int splash_frame(void) {
    if (!splash_active) return 0;

    splash_frame_count++;

    if (splash_frame_count > SPLASH_TOTAL_FRAMES) {
        splash_active = 0;
        ret_rend_clear_screen();
        RETRenderFrame();
        return 0;
    }

    // Calculate brightness based on fade phase
    float alpha;
    if (splash_frame_count <= SPLASH_FADE_IN_FRAMES) {
        alpha = (float)splash_frame_count / (float)SPLASH_FADE_IN_FRAMES;
    }
    else if (splash_frame_count <= SPLASH_FADE_IN_FRAMES + SPLASH_HOLD_FRAMES) {
        alpha = 1.0f;
    }
    else {
        int fade_frame = splash_frame_count - SPLASH_FADE_IN_FRAMES - SPLASH_HOLD_FRAMES;
        alpha = 1.0f - (float)fade_frame / (float)SPLASH_FADE_OUT_FRAMES;
    }

    // Map alpha to brightness index (0-15)
    int brightness = (int)(alpha * 15.0f);
    if (brightness < 0) brightness = 0;
    if (brightness > 15) brightness = 15;

    // Clear screen
    ret_rend_clear_screen();

    // Set green color for phosphor look
    byte old_fg = RETSetFgColor(RET_COLOR_GREEN);
    byte old_bright = RETSetFgBrightness((unsigned char)brightness);

    // Layout (26 rows total):
    // Row 1:      Apple logo starts (16 rows tall)
    // Row 18:     "50 YEARS OF APPLE"
    // Row 20:     "APPLE COMPUTER INC"
    // Row 22:     "1976 - 2026"
    // Row 24:     "CUPERTINO, CALIFORNIA"

    splash_draw_logo(1);
    splash_draw_centered("50 YEARS OF APPLE", 18);
    splash_draw_centered("APPLE COMPUTER INC", 20);
    splash_draw_centered("1976 - 2026", 22);
    splash_draw_centered("CUPERTINO CALIFORNIA", 24);

    // Restore colors
    RETSetFgColor(old_fg);
    RETSetFgBrightness(old_bright);

    RETRenderFrame();

    return 1;
}
