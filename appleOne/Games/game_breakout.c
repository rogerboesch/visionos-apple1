#include "game_breakout.h"
#include "rb_display.h"

#include <string.h>

/* -------------------------------------------------------------------------- */
/*  Brick state                                                               */
/* -------------------------------------------------------------------------- */

typedef struct {
    int alive;
    int x;
    int y;
    int row;
} breakout_brick;

/* -------------------------------------------------------------------------- */
/*  Game state                                                                */
/* -------------------------------------------------------------------------- */

static int game_state;
static int game_score;
static int game_lives;
static int game_level;

/* Paddle */
static int paddle_x;

/* Ball */
static int ball_x;
static int ball_y;
static int ball_dx;
static int ball_dy;
static int ball_stuck;

/* Ball speed constants */
#define BALL_SPEED_X   1
#define BALL_SPEED_Y  -1
#define BALL_MAX_DX    2
#define BALL_MAX_DY    2

/* Bricks */
static breakout_brick bricks[BREAKOUT_BRICK_COLS * BREAKOUT_BRICK_ROWS];
static int bricks_alive;

/* Input */
static int input_action;
static int input_held;

/* Play area boundaries */
#define PLAY_LEFT   (BREAKOUT_SIDE_W)
#define PLAY_RIGHT  (BREAKOUT_SCREEN_W - BREAKOUT_SIDE_W)
#define PLAY_TOP    (BREAKOUT_WALL_TOP + BREAKOUT_WALL_THICK)

/* Brick grid origin */
#define BRICK_ORIGIN_X  9
#define BRICK_ORIGIN_Y  30

/* Score per row (higher rows = more points) */
static const int row_scores[BREAKOUT_BRICK_ROWS] = {
    7, 7, 5, 5, 3, 3, 1
};

/* -------------------------------------------------------------------------- */
/*  Build brick grid                                                          */
/* -------------------------------------------------------------------------- */

static void build_bricks(void) {
    bricks_alive = 0;

    for (int row = 0; row < BREAKOUT_BRICK_ROWS; row++) {
        for (int col = 0; col < BREAKOUT_BRICK_COLS; col++) {
            int idx = row * BREAKOUT_BRICK_COLS + col;

            bricks[idx].alive = 1;
            bricks[idx].x = BRICK_ORIGIN_X + col * (BREAKOUT_BRICK_W + BREAKOUT_BRICK_GAP);
            bricks[idx].y = BRICK_ORIGIN_Y + row * (BREAKOUT_BRICK_H + BREAKOUT_BRICK_GAP);
            bricks[idx].row = row;
            bricks_alive++;
        }
    }
}

/* -------------------------------------------------------------------------- */
/*  Reset ball onto paddle                                                    */
/* -------------------------------------------------------------------------- */

static void reset_ball(void) {
    ball_x = paddle_x + BREAKOUT_PADDLE_W / 2 - BREAKOUT_BALL_SIZE / 2;
    ball_y = BREAKOUT_PADDLE_Y - BREAKOUT_BALL_SIZE;
    ball_dx = BALL_SPEED_X;
    ball_dy = BALL_SPEED_Y;
    ball_stuck = 1;
}

/* -------------------------------------------------------------------------- */
/*  Init / Reset                                                              */
/* -------------------------------------------------------------------------- */

void game_breakout_init(void) {
    game_state = BREAKOUT_STATE_IDLE;
    game_score = 0;
    game_lives = BREAKOUT_INITIAL_LIVES;
    game_level = 1;

    paddle_x = BREAKOUT_SCREEN_W / 2 - BREAKOUT_PADDLE_W / 2;

    build_bricks();
    reset_ball();

    input_action = BREAKOUT_INPUT_NONE;
    input_held = 0;
}

void game_breakout_reset(void) {
    game_breakout_init();
}

/* -------------------------------------------------------------------------- */
/*  Input                                                                     */
/* -------------------------------------------------------------------------- */

void game_breakout_input(int action) {
    input_action = action;
    input_held = 1;
}

void game_breakout_input_release(void) {
    input_action = BREAKOUT_INPUT_NONE;
    input_held = 0;
}

/* -------------------------------------------------------------------------- */
/*  Clamp helper                                                              */
/* -------------------------------------------------------------------------- */

static int clamp(int val, int lo, int hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

/* -------------------------------------------------------------------------- */
/*  Handle input                                                              */
/* -------------------------------------------------------------------------- */

static void handle_input(void) {
    if (game_state == BREAKOUT_STATE_IDLE) {
        if (input_action == BREAKOUT_INPUT_LAUNCH) {
            game_state = BREAKOUT_STATE_PLAYING;
            ball_stuck = 0;
            input_action = BREAKOUT_INPUT_NONE;
            input_held = 0;
        }
        return;
    }

    if (game_state == BREAKOUT_STATE_GAMEOVER) {
        return;
    }

    /* Playing state */
    if (input_action == BREAKOUT_INPUT_LEFT) {
        paddle_x -= BREAKOUT_PADDLE_SPEED;
    }
    else if (input_action == BREAKOUT_INPUT_RIGHT) {
        paddle_x += BREAKOUT_PADDLE_SPEED;
    }
    else if (input_action == BREAKOUT_INPUT_LAUNCH && ball_stuck) {
        ball_stuck = 0;
    }

    paddle_x = clamp(paddle_x, PLAY_LEFT, PLAY_RIGHT - BREAKOUT_PADDLE_W);

    /* Consume single-tap input (not held) */
    if (!input_held) {
        input_action = BREAKOUT_INPUT_NONE;
    }
}

/* -------------------------------------------------------------------------- */
/*  AABB overlap test                                                         */
/* -------------------------------------------------------------------------- */

static int aabb_overlap(int ax, int ay, int aw, int ah,
                        int bx, int by, int bw, int bh) {
    return (ax < bx + bw) && (ax + aw > bx) &&
           (ay < by + bh) && (ay + ah > by);
}

/* -------------------------------------------------------------------------- */
/*  Physics step (single substep)                                             */
/* -------------------------------------------------------------------------- */

static void step_physics(void) {
    if (ball_stuck) {
        ball_x = paddle_x + BREAKOUT_PADDLE_W / 2 - BREAKOUT_BALL_SIZE / 2;
        ball_y = BREAKOUT_PADDLE_Y - BREAKOUT_BALL_SIZE;
        return;
    }

    int nx = ball_x + ball_dx;
    int ny = ball_y + ball_dy;

    /* Wall collisions */
    if (nx <= PLAY_LEFT) {
        nx = PLAY_LEFT;
        ball_dx = -ball_dx;
    }
    else if (nx + BREAKOUT_BALL_SIZE >= PLAY_RIGHT) {
        nx = PLAY_RIGHT - BREAKOUT_BALL_SIZE;
        ball_dx = -ball_dx;
    }

    if (ny <= PLAY_TOP) {
        ny = PLAY_TOP;
        ball_dy = -ball_dy;
    }

    /* Ball fell below screen */
    if (ny >= BREAKOUT_SCREEN_H) {
        game_lives--;

        if (game_lives <= 0) {
            game_state = BREAKOUT_STATE_GAMEOVER;
            return;
        }

        reset_ball();
        return;
    }

    /* Paddle collision */
    if (ball_dy > 0 &&
        aabb_overlap(nx, ny, BREAKOUT_BALL_SIZE, BREAKOUT_BALL_SIZE,
                     paddle_x, BREAKOUT_PADDLE_Y, BREAKOUT_PADDLE_W, BREAKOUT_PADDLE_H)) {
        ny = BREAKOUT_PADDLE_Y - BREAKOUT_BALL_SIZE;
        ball_dy = -ball_dy;

        /* Spin based on hit offset from paddle center */
        int hit_center = nx + BREAKOUT_BALL_SIZE / 2;
        int paddle_center = paddle_x + BREAKOUT_PADDLE_W / 2;
        int offset = hit_center - paddle_center;

        ball_dx = offset / 10;
        if (ball_dx == 0) {
            ball_dx = (nx < paddle_center) ? -1 : 1;
        }

        ball_dx = clamp(ball_dx, -BALL_MAX_DX, BALL_MAX_DX);
    }

    /* Brick collisions */
    for (int i = 0; i < BREAKOUT_BRICK_COLS * BREAKOUT_BRICK_ROWS; i++) {
        if (!bricks[i].alive) continue;

        if (aabb_overlap(nx, ny, BREAKOUT_BALL_SIZE, BREAKOUT_BALL_SIZE,
                         bricks[i].x, bricks[i].y, BREAKOUT_BRICK_W, BREAKOUT_BRICK_H)) {
            bricks[i].alive = 0;
            bricks_alive--;
            game_score += row_scores[bricks[i].row];

            /* Determine reflection axis */
            int ball_cx = nx + BREAKOUT_BALL_SIZE / 2;
            int ball_cy = ny + BREAKOUT_BALL_SIZE / 2;
            int brick_cx = bricks[i].x + BREAKOUT_BRICK_W / 2;
            int brick_cy = bricks[i].y + BREAKOUT_BRICK_H / 2;

            int dx_dist = ball_cx - brick_cx;
            int dy_dist = ball_cy - brick_cy;

            /* Scale by aspect ratio for proper axis detection */
            if (dx_dist < 0) dx_dist = -dx_dist;
            if (dy_dist < 0) dy_dist = -dy_dist;

            int scaled_dx = dx_dist * BREAKOUT_BRICK_H;
            int scaled_dy = dy_dist * BREAKOUT_BRICK_W;

            if (scaled_dx > scaled_dy) {
                ball_dx = -ball_dx;
            }
            else {
                ball_dy = -ball_dy;
            }

            /* Only hit one brick per substep */
            break;
        }
    }

    /* All bricks cleared — next level */
    if (bricks_alive <= 0) {
        game_level++;
        build_bricks();
        reset_ball();
        ball_stuck = 0;
        return;
    }

    /* Clamp speed */
    ball_dx = clamp(ball_dx, -BALL_MAX_DX, BALL_MAX_DX);
    ball_dy = clamp(ball_dy, -BALL_MAX_DY, BALL_MAX_DY);

    /* Ensure ball never has zero vertical speed */
    if (ball_dy == 0) {
        ball_dy = -1;
    }

    ball_x = nx;
    ball_y = ny;
}

/* -------------------------------------------------------------------------- */
/*  Frame (called 60x/sec)                                                    */
/* -------------------------------------------------------------------------- */

int game_breakout_frame(void) {
    handle_input();

    if (game_state == BREAKOUT_STATE_IDLE) {
        game_breakout_render_title();
        rb_display_render_frame(BREAKOUT_DISPLAY);
        return 0;
    }

    if (game_state == BREAKOUT_STATE_GAMEOVER) {
        game_breakout_render_gameover(game_score);
        rb_display_render_frame(BREAKOUT_DISPLAY);
        return 0;
    }

    /* Run physics substeps */
    for (int i = 0; i < BREAKOUT_PHYSICS_STEPS; i++) {
        step_physics();
        if (game_state != BREAKOUT_STATE_PLAYING) break;
    }

    game_breakout_render_frame();
    rb_display_render_frame(BREAKOUT_DISPLAY);

    return 0;
}

/* -------------------------------------------------------------------------- */
/*  Getters                                                                   */
/* -------------------------------------------------------------------------- */

int game_breakout_get_score(void) { return game_score; }
int game_breakout_get_lives(void) { return game_lives; }
int game_breakout_get_state(void) { return game_state; }

/* -------------------------------------------------------------------------- */
/*  Accessors for render module                                               */
/* -------------------------------------------------------------------------- */

int  game_breakout_get_paddle_x(void)  { return paddle_x; }
int  game_breakout_get_ball_x(void)    { return ball_x; }
int  game_breakout_get_ball_y(void)    { return ball_y; }
int  game_breakout_get_ball_stuck(void){ return ball_stuck; }
int  game_breakout_get_level(void)     { return game_level; }

const breakout_brick *game_breakout_get_bricks(void) { return bricks; }
