
#include "splash.h"
#include "rb_display.h"

#include <string.h>

/* Screen grid: 42 columns x 26 rows (8x8 font, 336x208 pixels) */
#define SPLASH_COLS  42
#define SPLASH_ROWS  26

#define SPLASH_DISPLAY 0

/* Animation timing (in frames at 60 fps) */
#define FADE_IN_FRAMES      45   /* 0.75 seconds */
#define FADE_OUT_FRAMES     45   /* 0.75 seconds */
#define LOGO_ROW_SPEED       5   /* frames per logo row */
#define TYPE_SPEED           6   /* frames per character */
#define SEPARATOR_SPEED      1   /* frames per '=' character */
#define CURSOR_BLINK_RATE   20   /* frames per blink toggle */
#define PAUSE_SHORT         15   /* 0.25 seconds */
#define PAUSE_MEDIUM        20   /* 0.33 seconds */

/* Animation phases */
enum {
    PHASE_FADE_IN = 0,
    PHASE_LOGO,
    PHASE_PAUSE_AFTER_LOGO,
    PHASE_SEPARATOR_1,
    PHASE_PAUSE_1,
    PHASE_TYPE_LINE_1,
    PHASE_TYPE_LINE_2,
    PHASE_TYPE_LINE_3,
    PHASE_PAUSE_2,
    PHASE_SEPARATOR_2,
    PHASE_PAUSE_3,
    PHASE_TYPE_LINE_4,
    PHASE_PAUSE_4,
    PHASE_HOLD,
    PHASE_FADE_OUT,
    PHASE_COUNT
};

/* Apple logo (12 rows, compact) */
#define LOGO_ROWS 12
#define LOGO_COLS 12

static const char *apple_logo[LOGO_ROWS] = {
    "     ##     ",
    "    ###     ",
    " ######### ",
    "###########",
    "############",
    "############",
    "############",
    "############",
    " ###########",
    " ########## ",
    "  ########  ",
    "   ######   ",
};

/* Text content */
#define SEPARATOR_LEN  38

static const char *type_lines[] = {
    "APPLE COMPUTER INC",
    "50 YEARS OF INNOVATION",
    "1976 - 2026",
    "CUPERTINO  .  CALIFORNIA",
};

static const int type_rows[] = { 16, 17, 18, 22 };

#define TYPE_LINE_COUNT 4

/* Woz monitor prompt */
#define PROMPT_ROW  24
#define PROMPT_COL   0
#define PROMPT_CHAR '\\'

/* Separator rows */
#define SEP_ROW_1   14
#define SEP_ROW_2   20

/* State */
static int splash_active;
static int splash_phase;
static int splash_phase_frame;
static int splash_type_index;
static int splash_cursor_blink;
static int splash_logo_rows_drawn;
static int splash_brightness;

/* --- helpers --- */

static int splash_center_col(int len) {
    return (SPLASH_COLS - len) / 2;
}

static void splash_draw_char(int row, int col, char ch) {
    byte color = rb_display_get_fg_color(SPLASH_DISPLAY);
    rb_display_text_print_char(SPLASH_DISPLAY, row, col, ch, color);
}

static void splash_draw_string(const char *str, int row, int col) {
    for (int i = 0; str[i]; i++) {
        splash_draw_char(row, col + i, str[i]);
    }
}

static void splash_draw_centered(const char *str, int row) {
    int len = (int)strlen(str);
    int col = splash_center_col(len);
    splash_draw_string(str, row, col);
}

static void splash_draw_logo_row(int logo_row) {
    int col_offset = (SPLASH_COLS - LOGO_COLS) / 2;
    byte color = rb_display_get_fg_color(SPLASH_DISPLAY);

    rb_display_set_invert(SPLASH_DISPLAY, true);

    const char *line = apple_logo[logo_row];
    int len = (int)strlen(line);
    for (int col = 0; col < len; col++) {
        if (line[col] == '#') {
            rb_display_text_print_char(SPLASH_DISPLAY, 1 + logo_row,
                                       col_offset + col, ' ', color);
        }
    }

    rb_display_set_invert(SPLASH_DISPLAY, false);
}

/* Draw all logo rows revealed so far */
static void splash_draw_logo_all(void) {
    for (int r = 0; r < splash_logo_rows_drawn; r++) {
        splash_draw_logo_row(r);
    }
}

/* Draw partial separator (up to n characters) */
static void splash_draw_separator(int row, int count) {
    int col = splash_center_col(SEPARATOR_LEN);
    for (int i = 0; i < count && i < SEPARATOR_LEN; i++) {
        splash_draw_char(row, col + i, '=');
    }
}

/* Draw partial typed line (up to n characters) */
static void splash_draw_typed_line(int line_idx, int count) {
    const char *str = type_lines[line_idx];
    int len = (int)strlen(str);
    int col = splash_center_col(len);
    for (int i = 0; i < count && i < len; i++) {
        splash_draw_char(type_rows[line_idx], col + i, str[i]);
    }
}

/* Draw cursor (inverted space block) at a position */
static void splash_draw_cursor_at(int row, int col) {
    byte color = rb_display_get_fg_color(SPLASH_DISPLAY);
    rb_display_set_invert(SPLASH_DISPLAY, true);
    rb_display_text_print_char(SPLASH_DISPLAY, row, col, ' ', color);
    rb_display_set_invert(SPLASH_DISPLAY, false);
}

/* Get cursor position for a typing line */
static void splash_get_type_cursor(int line_idx, int char_pos,
                                   int *out_row, int *out_col) {
    const char *str = type_lines[line_idx];
    int len = (int)strlen(str);
    int start_col = splash_center_col(len);
    *out_row = type_rows[line_idx];
    *out_col = start_col + char_pos;
}

/* Get cursor position for a separator */
static void splash_get_sep_cursor(int sep_row, int char_pos,
                                  int *out_row, int *out_col) {
    int start_col = splash_center_col(SEPARATOR_LEN);
    *out_row = sep_row;
    *out_col = start_col + char_pos;
}

/* Draw everything that's been revealed up to the current phase */
static void splash_draw_content(int cursor_visible) {
    /* Logo */
    splash_draw_logo_all();

    /* Separator 1 (fully drawn once past phase) */
    if (splash_phase > PHASE_SEPARATOR_1) {
        splash_draw_separator(SEP_ROW_1, SEPARATOR_LEN);
    }
    else if (splash_phase == PHASE_SEPARATOR_1) {
        splash_draw_separator(SEP_ROW_1, splash_type_index);
    }

    /* Type lines 1-3 */
    for (int i = 0; i < 3; i++) {
        int line_phase = PHASE_TYPE_LINE_1 + i;
        int len = (int)strlen(type_lines[i]);
        if (splash_phase > line_phase) {
            splash_draw_typed_line(i, len);
        }
        else if (splash_phase == line_phase) {
            splash_draw_typed_line(i, splash_type_index);
        }
    }

    /* Separator 2 */
    if (splash_phase > PHASE_SEPARATOR_2) {
        splash_draw_separator(SEP_ROW_2, SEPARATOR_LEN);
    }
    else if (splash_phase == PHASE_SEPARATOR_2) {
        splash_draw_separator(SEP_ROW_2, splash_type_index);
    }

    /* Type line 4 */
    {
        int len = (int)strlen(type_lines[3]);
        if (splash_phase > PHASE_TYPE_LINE_4) {
            splash_draw_typed_line(3, len);
        }
        else if (splash_phase == PHASE_TYPE_LINE_4) {
            splash_draw_typed_line(3, splash_type_index);
        }
    }

    /* Woz monitor prompt (shown in hold and fade-out) */
    if (splash_phase >= PHASE_HOLD) {
        splash_draw_char(PROMPT_ROW, PROMPT_COL, PROMPT_CHAR);
    }

    /* Blinking cursor */
    if (cursor_visible) {
        int crow, ccol;

        if (splash_phase == PHASE_SEPARATOR_1) {
            splash_get_sep_cursor(SEP_ROW_1, splash_type_index, &crow, &ccol);
            splash_draw_cursor_at(crow, ccol);
        }
        else if (splash_phase >= PHASE_TYPE_LINE_1 &&
                 splash_phase <= PHASE_TYPE_LINE_3) {
            int idx = splash_phase - PHASE_TYPE_LINE_1;
            splash_get_type_cursor(idx, splash_type_index, &crow, &ccol);
            splash_draw_cursor_at(crow, ccol);
        }
        else if (splash_phase == PHASE_SEPARATOR_2) {
            splash_get_sep_cursor(SEP_ROW_2, splash_type_index, &crow, &ccol);
            splash_draw_cursor_at(crow, ccol);
        }
        else if (splash_phase == PHASE_TYPE_LINE_4) {
            splash_get_type_cursor(3, splash_type_index, &crow, &ccol);
            splash_draw_cursor_at(crow, ccol);
        }
        else if (splash_phase >= PHASE_HOLD) {
            /* Cursor after prompt */
            splash_draw_cursor_at(PROMPT_ROW, PROMPT_COL + 1);
        }
    }
}

static void splash_advance_phase(void) {
    splash_phase++;
    splash_phase_frame = 0;
    splash_type_index = 0;
}

/* --- public API --- */

void splash_init(void) {
    splash_active = 1;
    splash_phase = PHASE_FADE_IN;
    splash_phase_frame = 0;
    splash_type_index = 0;
    splash_cursor_blink = 0;
    splash_logo_rows_drawn = 0;
    splash_brightness = 0;
}

void splash_skip(void) {
    if (!splash_active) return;
    if (splash_phase == PHASE_FADE_OUT) return;

    splash_phase = PHASE_FADE_OUT;
    splash_phase_frame = 0;
}

int splash_frame(void) {
    if (!splash_active) return 0;

    splash_phase_frame++;
    splash_cursor_blink++;

    int cursor_on = ((splash_cursor_blink / CURSOR_BLINK_RATE) % 2) == 0;

    /* Phase logic */
    switch (splash_phase) {

    case PHASE_FADE_IN:
        splash_brightness = (splash_phase_frame * 15) / FADE_IN_FRAMES;
        if (splash_brightness > 15) splash_brightness = 15;
        if (splash_phase_frame >= FADE_IN_FRAMES) {
            splash_brightness = 15;
            splash_advance_phase();
        }
        break;

    case PHASE_LOGO:
        if (splash_phase_frame % LOGO_ROW_SPEED == 0 &&
            splash_logo_rows_drawn < LOGO_ROWS) {
            splash_logo_rows_drawn++;
        }
        if (splash_logo_rows_drawn >= LOGO_ROWS) {
            splash_advance_phase();
        }
        break;

    case PHASE_PAUSE_AFTER_LOGO:
        if (splash_phase_frame >= PAUSE_MEDIUM) {
            splash_advance_phase();
        }
        break;

    case PHASE_SEPARATOR_1:
        if (splash_phase_frame % SEPARATOR_SPEED == 0 &&
            splash_type_index < SEPARATOR_LEN) {
            splash_type_index++;
        }
        if (splash_type_index >= SEPARATOR_LEN) {
            splash_advance_phase();
        }
        break;

    case PHASE_PAUSE_1:
        if (splash_phase_frame >= PAUSE_SHORT) {
            splash_advance_phase();
        }
        break;

    case PHASE_TYPE_LINE_1:
    case PHASE_TYPE_LINE_2:
    case PHASE_TYPE_LINE_3: {
        int idx = splash_phase - PHASE_TYPE_LINE_1;
        int len = (int)strlen(type_lines[idx]);
        if (splash_phase_frame % TYPE_SPEED == 0 &&
            splash_type_index < len) {
            splash_type_index++;
        }
        if (splash_type_index >= len) {
            splash_advance_phase();
        }
        break;
    }

    case PHASE_PAUSE_2:
        if (splash_phase_frame >= PAUSE_SHORT) {
            splash_advance_phase();
        }
        break;

    case PHASE_SEPARATOR_2:
        if (splash_phase_frame % SEPARATOR_SPEED == 0 &&
            splash_type_index < SEPARATOR_LEN) {
            splash_type_index++;
        }
        if (splash_type_index >= SEPARATOR_LEN) {
            splash_advance_phase();
        }
        break;

    case PHASE_PAUSE_3:
        if (splash_phase_frame >= PAUSE_SHORT) {
            splash_advance_phase();
        }
        break;

    case PHASE_TYPE_LINE_4: {
        int len = (int)strlen(type_lines[3]);
        if (splash_phase_frame % TYPE_SPEED == 0 &&
            splash_type_index < len) {
            splash_type_index++;
        }
        if (splash_type_index >= len) {
            splash_advance_phase();
        }
        break;
    }

    case PHASE_PAUSE_4:
        if (splash_phase_frame >= PAUSE_SHORT) {
            splash_advance_phase();
        }
        break;

    case PHASE_HOLD:
        /* Hold indefinitely until splash_skip() triggers fade-out */
        break;

    case PHASE_FADE_OUT: {
        int remaining = FADE_OUT_FRAMES - splash_phase_frame;
        splash_brightness = (remaining * 15) / FADE_OUT_FRAMES;
        if (splash_brightness < 0) splash_brightness = 0;
        if (splash_phase_frame >= FADE_OUT_FRAMES) {
            splash_active = 0;
            rb_display_render_clear(SPLASH_DISPLAY);
            rb_display_render_frame(SPLASH_DISPLAY);
            return 0;
        }
        break;
    }

    default:
        break;
    }

    /* Render frame */
    rb_display_render_clear(SPLASH_DISPLAY);

    byte old_fg = rb_display_set_fg_color(SPLASH_DISPLAY, RB_COLOR_GREEN);
    byte old_bright = rb_display_set_fg_brightness(SPLASH_DISPLAY,
                                                   (byte)splash_brightness);
    rb_display_text_set_immediate(SPLASH_DISPLAY, 1);

    splash_draw_content(cursor_on);

    rb_display_set_fg_color(SPLASH_DISPLAY, old_fg);
    rb_display_set_fg_brightness(SPLASH_DISPLAY, old_bright);

    rb_display_render_frame(SPLASH_DISPLAY);

    return 1;
}
