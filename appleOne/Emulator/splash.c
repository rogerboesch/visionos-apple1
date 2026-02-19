
#include "splash.h"
#include "ret_renderer.h"
#include "ret_textbuffer.h"

#include <string.h>

// Splash timing (in frames at 60 fps)
#define SPLASH_FADE_IN_FRAMES   90   // 1.5 seconds
#define SPLASH_HOLD_FRAMES      120  // 2.0 seconds
#define SPLASH_FADE_OUT_FRAMES  45   // 0.75 seconds
#define SPLASH_TOTAL_FRAMES     (SPLASH_FADE_IN_FRAMES + SPLASH_HOLD_FRAMES + SPLASH_FADE_OUT_FRAMES)

static int splash_frame_count = 0;
static int splash_active = 0;

// Scale factor for the "50" text
#define SCALE 4

// Draw a single vector font character scaled up
static void draw_scaled_char(char ch, int x, int y, int scale) {
    extern const unsigned char ret_vecfont[128][8];

    if (ch < 0x20 || ch > 127) return;

    const unsigned char *p = ret_vecfont[ch - 0x20];
    int cx = 0, cy = 0;
    int bright = 0;

    for (int i = 0; i < 8; i++) {
        unsigned char b = p[i];

        if (b == 0xFF) break;      // FONT_LAST
        if (b == 0xFE) {           // FONT_UP
            bright = 0;
            continue;
        }

        int x2 = (b >> 4) & 0x0F;
        int y2 = b & 0x0F;

        if (bright) {
            // Draw line from (cx,cy) to (x2,y2) scaled
            int sx1 = x + cx * scale;
            int sy1 = y - cy * scale;
            int sx2 = x + x2 * scale;
            int sy2 = y - y2 * scale;
            ret_rend_draw_line(sx1, sy1, sx2, sy2,
                RETGetFgColor(), RETGetFgBrightness());
        }

        bright = 1;
        cx = x2;
        cy = y2;
    }
}

// Draw a string of scaled vector characters
static void draw_scaled_string(const char *str, int x, int y, int scale) {
    int char_width = (8 + 2) * scale;  // 8 units + 2 spacing

    while (*str) {
        draw_scaled_char(*str, x, y, scale);
        x += char_width;
        str++;
    }
}

// Get pixel width of a scaled string
static int scaled_string_width(const char *str, int scale) {
    int len = (int)strlen(str);
    if (len == 0) return 0;
    return len * (8 + 2) * scale - 2 * scale;  // No trailing space
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

    int screen_w = RET_PIXEL_WIDTH;
    int screen_h = RET_PIXEL_HEIGHT;

    // Draw "50" big and centered
    const char *text_50 = "50";
    int w50 = scaled_string_width(text_50, SCALE);
    int x50 = (screen_w - w50) / 2;
    int y50 = screen_h / 2 + 12 * SCALE;  // Vector font draws downward from baseline
    draw_scaled_string(text_50, x50, y50, SCALE);

    // Draw "YEARS OF APPLE" smaller below
    const char *text_years = "YEARS OF APPLE";
    int w_years = scaled_string_width(text_years, 1);
    int x_years = (screen_w - w_years) / 2;
    int y_years = y50 + 20;
    RETDrawVString((char *)text_years, x_years, y_years - 12);

    // Draw "1976 - 2026" at the bottom
    const char *text_dates = "1976 - 2026";
    int w_dates = scaled_string_width(text_dates, 1);
    int x_dates = (screen_w - w_dates) / 2;
    int y_dates = screen_h - 20;
    RETDrawVString((char *)text_dates, x_dates, y_dates - 12);

    // Restore colors
    RETSetFgColor(old_fg);
    RETSetFgBrightness(old_bright);

    RETRenderFrame();

    return 1;
}
