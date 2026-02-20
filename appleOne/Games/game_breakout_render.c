#include "game_breakout.h"
#include "rb_display.h"

#include <stdio.h>
#include <string.h>

/* Access to display struct (same pattern as rb_display_render.c) */
extern rb_display *rb_get_display(int index);

/* -------------------------------------------------------------------------- */
/*  Accessors from game_breakout.c                                            */
/* -------------------------------------------------------------------------- */

extern int  game_breakout_get_paddle_x(void);
extern int  game_breakout_get_ball_x(void);
extern int  game_breakout_get_ball_y(void);
extern int  game_breakout_get_ball_stuck(void);
extern int  game_breakout_get_level(void);

typedef struct {
    int alive;
    int x;
    int y;
    int row;
} breakout_brick;

extern const breakout_brick *game_breakout_get_bricks(void);

/* -------------------------------------------------------------------------- */
/*  Color constants (palette + brightness)                                    */
/* -------------------------------------------------------------------------- */

#define COLOR_WALL       RB_COLOR_GREEN
#define BRIGHT_WALL      4

#define COLOR_PADDLE     RB_COLOR_GREEN
#define BRIGHT_PADDLE    RB_BRIGHTNESS_NORMAL

#define COLOR_BALL       RB_COLOR_WHITE
#define BRIGHT_BALL      RB_BRIGHTNESS_NORMAL

#define COLOR_HUD        RB_COLOR_GREEN
#define BRIGHT_HUD       6

/* Brick brightness per row (top = brightest, bottom = dimmest) */
static const byte brick_brightness[BREAKOUT_BRICK_ROWS] = {
    10, 9, 8, 7, 6, 5, 4
};

#define COLOR_BRICK      RB_COLOR_GREEN

/* Title/gameover text colors */
#define COLOR_TITLE      RB_COLOR_GREEN
#define BRIGHT_TITLE     RB_BRIGHTNESS_NORMAL

/* -------------------------------------------------------------------------- */
/*  fill_rect — write directly to pixel_data                                  */
/* -------------------------------------------------------------------------- */

static void fill_rect(int x, int y, int w, int h,
                      byte palette_color, byte brightness) {
    rb_display *d = rb_get_display(BREAKOUT_DISPLAY);
    int pw = d->pixel_width;
    int ph = d->pixel_height;
    byte *pixels = d->pixel_data;

    rb_color c = rb_display_palette_get_color_brightness(palette_color, brightness);

    /* Clip */
    int x0 = (x < 0) ? 0 : x;
    int y0 = (y < 0) ? 0 : y;
    int x1 = (x + w > pw) ? pw : x + w;
    int y1 = (y + h > ph) ? ph : y + h;

    for (int iy = y0; iy < y1; iy++) {
        int row_offset = iy * pw * 4;
        for (int ix = x0; ix < x1; ix++) {
            int off = row_offset + ix * 4;
            pixels[off]     = c.r;
            pixels[off + 1] = c.g;
            pixels[off + 2] = c.b;
            pixels[off + 3] = 255;
        }
    }
}

/* -------------------------------------------------------------------------- */
/*  Draw helpers                                                              */
/* -------------------------------------------------------------------------- */

static void draw_walls(void) {
    /* Top wall */
    fill_rect(0, BREAKOUT_WALL_TOP,
              BREAKOUT_SCREEN_W, BREAKOUT_WALL_THICK,
              COLOR_WALL, BRIGHT_WALL);

    /* Left wall */
    fill_rect(0, BREAKOUT_WALL_TOP,
              BREAKOUT_SIDE_W, BREAKOUT_SCREEN_H - BREAKOUT_WALL_TOP,
              COLOR_WALL, BRIGHT_WALL);

    /* Right wall */
    fill_rect(BREAKOUT_SCREEN_W - BREAKOUT_SIDE_W, BREAKOUT_WALL_TOP,
              BREAKOUT_SIDE_W, BREAKOUT_SCREEN_H - BREAKOUT_WALL_TOP,
              COLOR_WALL, BRIGHT_WALL);
}

static void draw_bricks(void) {
    const breakout_brick *b = game_breakout_get_bricks();
    int total = BREAKOUT_BRICK_COLS * BREAKOUT_BRICK_ROWS;

    for (int i = 0; i < total; i++) {
        if (!b[i].alive) continue;

        fill_rect(b[i].x, b[i].y,
                  BREAKOUT_BRICK_W, BREAKOUT_BRICK_H,
                  COLOR_BRICK, brick_brightness[b[i].row]);
    }
}

static void draw_paddle(void) {
    int px = game_breakout_get_paddle_x();

    fill_rect(px, BREAKOUT_PADDLE_Y,
              BREAKOUT_PADDLE_W, BREAKOUT_PADDLE_H,
              COLOR_PADDLE, BRIGHT_PADDLE);
}

static void draw_ball(void) {
    int bx = game_breakout_get_ball_x();
    int by = game_breakout_get_ball_y();

    fill_rect(bx, by,
              BREAKOUT_BALL_SIZE, BREAKOUT_BALL_SIZE,
              COLOR_BALL, BRIGHT_BALL);
}

/* -------------------------------------------------------------------------- */
/*  Draw HUD (score, lives, level)                                            */
/* -------------------------------------------------------------------------- */

static void draw_string(int x, int y, const char *str, byte color, byte brightness) {
    byte old_bright = rb_display_set_fg_brightness(BREAKOUT_DISPLAY, brightness);

    while (*str) {
        rb_display_render_draw_char(BREAKOUT_DISPLAY, x, y, *str, 0, color);
        x += RB_FONT_WIDTH;
        str++;
    }

    rb_display_set_fg_brightness(BREAKOUT_DISPLAY, old_bright);
}

static void int_to_str(int val, char *buf, int bufsize) {
    /* Simple integer to string (positive only) */
    if (val < 0) val = 0;

    char tmp[16];
    int pos = 0;

    if (val == 0) {
        tmp[pos++] = '0';
    }
    else {
        while (val > 0 && pos < 15) {
            tmp[pos++] = '0' + (val % 10);
            val /= 10;
        }
    }

    /* Reverse into buf */
    int i;
    for (i = 0; i < pos && i < bufsize - 1; i++) {
        buf[i] = tmp[pos - 1 - i];
    }
    buf[i] = '\0';
}

static void draw_hud(void) {
    char buf[32];

    /* Score (left) */
    draw_string(8, 4, "SCORE:", COLOR_HUD, BRIGHT_HUD);
    int_to_str(game_breakout_get_score(), buf, sizeof(buf));
    draw_string(8 + 6 * RB_FONT_WIDTH, 4, buf, COLOR_HUD, BRIGHT_HUD);

    /* Lives (center) */
    int_to_str(game_breakout_get_lives(), buf, sizeof(buf));
    draw_string(BREAKOUT_SCREEN_W / 2 - 3 * RB_FONT_WIDTH, 4,
                "LIVES:", COLOR_HUD, BRIGHT_HUD);
    draw_string(BREAKOUT_SCREEN_W / 2 + 3 * RB_FONT_WIDTH, 4,
                buf, COLOR_HUD, BRIGHT_HUD);

    /* Level (right) */
    draw_string(BREAKOUT_SCREEN_W - 14 * RB_FONT_WIDTH, 4,
                "LEVEL:", COLOR_HUD, BRIGHT_HUD);
    int_to_str(game_breakout_get_level(), buf, sizeof(buf));
    draw_string(BREAKOUT_SCREEN_W - 8 * RB_FONT_WIDTH, 4,
                buf, COLOR_HUD, BRIGHT_HUD);

    /* Separator line below HUD */
    fill_rect(0, BREAKOUT_HUD_H - 1, BREAKOUT_SCREEN_W, 1, COLOR_HUD, 3);
}

/* -------------------------------------------------------------------------- */
/*  Public: Render full game frame                                            */
/* -------------------------------------------------------------------------- */

void game_breakout_render_frame(void) {
    rb_display_render_clear(BREAKOUT_DISPLAY);

    draw_hud();
    draw_walls();
    draw_bricks();
    draw_paddle();
    draw_ball();
}

/* -------------------------------------------------------------------------- */
/*  Public: Title screen                                                      */
/* -------------------------------------------------------------------------- */

void game_breakout_render_title(void) {
    rb_display_render_clear(BREAKOUT_DISPLAY);

    draw_walls();

    /* Title text — centered */
    draw_string(BREAKOUT_SCREEN_W / 2 - 4 * RB_FONT_WIDTH, 60,
                "BREAKOUT", COLOR_TITLE, BRIGHT_TITLE);

    /* Instructions */
    draw_string(BREAKOUT_SCREEN_W / 2 - 8 * RB_FONT_WIDTH, 100,
                "PRESS LAUNCH TO START", COLOR_HUD, 5);

    /* Draw some decorative bricks */
    draw_bricks();
    draw_paddle();
    draw_ball();
}

/* -------------------------------------------------------------------------- */
/*  Public: Game over screen                                                  */
/* -------------------------------------------------------------------------- */

void game_breakout_render_gameover(int score) {
    rb_display_render_clear(BREAKOUT_DISPLAY);

    draw_walls();

    draw_string(BREAKOUT_SCREEN_W / 2 - 5 * RB_FONT_WIDTH, 60,
                "GAME  OVER", COLOR_TITLE, BRIGHT_TITLE);

    /* Show final score */
    char buf[32];
    draw_string(BREAKOUT_SCREEN_W / 2 - 4 * RB_FONT_WIDTH, 100,
                "SCORE:  ", COLOR_HUD, BRIGHT_HUD);
    int_to_str(score, buf, sizeof(buf));
    draw_string(BREAKOUT_SCREEN_W / 2 + 4 * RB_FONT_WIDTH, 100,
                buf, COLOR_HUD, BRIGHT_HUD);

    draw_string(BREAKOUT_SCREEN_W / 2 - 8 * RB_FONT_WIDTH, 140,
                "PRESS RESET TO RETRY", COLOR_HUD, 4);
}
