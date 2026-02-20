#ifndef GAME_BREAKOUT_H
#define GAME_BREAKOUT_H

/* Input actions */
#define BREAKOUT_INPUT_NONE    0
#define BREAKOUT_INPUT_LEFT    1
#define BREAKOUT_INPUT_RIGHT   2
#define BREAKOUT_INPUT_LAUNCH  3

/* Game states */
#define BREAKOUT_STATE_IDLE     0
#define BREAKOUT_STATE_PLAYING  1
#define BREAKOUT_STATE_GAMEOVER 2

/* Display constants */
#define BREAKOUT_DISPLAY       0
#define BREAKOUT_SCREEN_W    336
#define BREAKOUT_SCREEN_H    208

/* Playfield layout */
#define BREAKOUT_HUD_H        24
#define BREAKOUT_WALL_TOP     24
#define BREAKOUT_WALL_THICK    4
#define BREAKOUT_SIDE_W        4

#define BREAKOUT_BRICK_COLS   10
#define BREAKOUT_BRICK_ROWS    7
#define BREAKOUT_BRICK_W      30
#define BREAKOUT_BRICK_H       8
#define BREAKOUT_BRICK_GAP     2

#define BREAKOUT_PADDLE_W     40
#define BREAKOUT_PADDLE_H      4
#define BREAKOUT_PADDLE_Y    196

#define BREAKOUT_BALL_SIZE     4

#define BREAKOUT_INITIAL_LIVES 3
#define BREAKOUT_PADDLE_SPEED  5
#define BREAKOUT_PHYSICS_STEPS 3

/* Public API */
void game_breakout_init(void);
void game_breakout_reset(void);
int  game_breakout_frame(void);
void game_breakout_input(int action);
void game_breakout_input_release(void);
int  game_breakout_get_score(void);
int  game_breakout_get_lives(void);
int  game_breakout_get_state(void);

/* Render API (called from game_breakout.c) */
void game_breakout_render_frame(void);
void game_breakout_render_title(void);
void game_breakout_render_gameover(int score);

#endif
