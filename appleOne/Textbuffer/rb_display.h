#ifndef RB_DISPLAY_H
#define RB_DISPLAY_H

/* Datatypes */
#ifndef byte
	typedef unsigned char byte;
#endif

#ifndef boolean
	#define boolean int
	#define true 1
	#define false 0
#endif

/* Palette */
#define RB_PALETTE_SIZE 16
#define RB_BRIGHTNESS_NORMAL 7

#define RB_COLOR_BLACK         0
#define RB_COLOR_WHITE         1
#define RB_COLOR_RED           2
#define RB_COLOR_TURKEY        3
#define RB_COLOR_VIOLETT       4
#define RB_COLOR_GREEN         5
#define RB_COLOR_BLUE          6
#define RB_COLOR_YELLOW        7
#define RB_COLOR_ORANGE        8
#define RB_COLOR_BROWN         9
#define RB_COLOR_REDLIGHT      10
#define RB_COLOR_GRAY1         11
#define RB_COLOR_GRAY2         12
#define RB_COLOR_GREENLIGHT    13
#define RB_COLOR_BLUELIGHT     14
#define RB_COLOR_GRAY3         15

typedef struct rb_color_ {
	byte r, g, b;
} rb_color;

rb_color rb_display_palette_get_color(byte index);
rb_color rb_display_palette_get_color_brightness(byte index, byte bindex);

/* Font dimensions */
#define RB_FONT_WIDTH  8
#define RB_FONT_HEIGHT 8

#define RB_FONT_VEC_WIDTH  10
#define RB_FONT_VEC_HEIGHT 16
#define RB_VFONT_SPACE 2

/* Maximum number of simultaneous displays */
#define RB_DISPLAY_MAX 16

/* Standard colors */
#define RB_COLOR_BG        RB_COLOR_BLACK
#define RB_COLOR_FG        RB_COLOR_WHITE
#define RB_COLOR_CURSOR    RB_COLOR_BLUELIGHT

/* Color escape codes for inline color changes in print strings */
#define RB_ESCAPE_BASE_INDEX_FOR_COLOR 240
#define RB_ESCAPE_INVERT_ON  (RB_ESCAPE_BASE_INDEX_FOR_COLOR - 2)
#define RB_ESCAPE_INVERT_OFF (RB_ESCAPE_INVERT_ON + 1)

/* Vector font drawing helpers */
#define P(x,y) ((((x) & 0xF) << 4) | (((y) & 0xF) << 0))
#define FONT_UP   0xFE
#define FONT_LAST 0xFF

/* -------------------------------------------------------------------------- */
/*  Display struct                                                            */
/* -------------------------------------------------------------------------- */

typedef struct rb_display {
    /* Identity */
    int index;
    int active;

    /* Text buffer */
    int text_cols;
    int text_rows;
    byte *text_buffer;          /* text_cols * text_rows */
    byte *text_colbuf;          /* text_cols * text_rows */
    char *line_buffer;          /* text_cols */

    /* Pixel buffer (malloc'd: pixel_width * pixel_height * 4) */
    int pixel_width;
    int pixel_height;
    byte *pixel_data;

    /* Text cursor */
    int cursor_row;
    int cursor_col;
    int cursor_blink;
    int cursor_cycles;
    boolean cursor_visible;

    /* Color state (per-display) */
    byte bg_color;
    byte fg_color;
    byte fg_brightness;
    boolean invert_mode;

    /* Graphics cursor (for vector drawing) */
    int graphics_x;
    int graphics_y;
    byte mode_dotted;

    /* Rendering flags */
    boolean dirty;
    int immediate_print;        /* 0=buffered, 1=direct print */
    int direct_render;          /* 0=needs RenderFrame, 1=output every char */
} rb_display;

/* -------------------------------------------------------------------------- */
/*  Lifecycle                                                                 */
/* -------------------------------------------------------------------------- */

void rb_display_init(int cols, int rows);       /* Creates display 0 with given dimensions */
int  rb_display_create(int cols, int rows);     /* Returns index, malloc's buffers */
void rb_display_destroy(int display);           /* Free buffers, mark inactive */
void rb_display_set_current(int display);       /* Switch active display */
int  rb_display_get_current(void);

/* -------------------------------------------------------------------------- */
/*  Dimensions / access                                                       */
/* -------------------------------------------------------------------------- */

int  rb_display_get_text_width(int display);
int  rb_display_get_text_height(int display);
int  rb_display_get_pixel_width(int display);
int  rb_display_get_pixel_height(int display);
byte *rb_display_get_pixel_data(int display);

/* -------------------------------------------------------------------------- */
/*  Text operations (operate on current display)                              */
/* -------------------------------------------------------------------------- */

void rb_display_text_clear(void);
void rb_display_text_flush(void);
void rb_display_text_scroll_up(void);
void rb_display_text_set_immediate(int flag);
boolean rb_display_text_print_char(int row, int col, char ch, byte color);
char rb_display_text_get_char(int row, int col);
void rb_display_text_print_no_linebreak(char *str, int size);
const char *rb_display_text_get_line_buffer(void);

/* -------------------------------------------------------------------------- */
/*  Cursor (operate on current display)                                       */
/* -------------------------------------------------------------------------- */

void rb_display_cursor_show(boolean flag);
void rb_display_cursor_left(void);
void rb_display_cursor_right(void);
void rb_display_cursor_up(void);
void rb_display_cursor_down(void);
void rb_display_cursor_set(int x, int y);
void rb_display_cursor_blink(void);
void rb_display_cursor_draw(void);
void rb_display_cursor_delete_char(void);

/* -------------------------------------------------------------------------- */
/*  Rendering (operate on current display)                                    */
/* -------------------------------------------------------------------------- */

void rb_display_render_clear(void);
void rb_display_render_draw_char(int x, int y, char ch, int invert, byte color);
void rb_display_render_scroll_up(int height);
void rb_display_render_frame(void);
void rb_display_render_set_direct(int mode);

/* -------------------------------------------------------------------------- */
/*  Color state (per current display)                                         */
/* -------------------------------------------------------------------------- */

byte rb_display_set_bg_color(byte index);
byte rb_display_get_bg_color(void);
byte rb_display_set_fg_color(byte index);
byte rb_display_get_fg_color(void);
byte rb_display_set_fg_brightness(byte index);
byte rb_display_get_fg_brightness(void);
void rb_display_set_invert(boolean flag);
byte rb_display_set_dot_mode(byte mode);

/* -------------------------------------------------------------------------- */
/*  Graphics cursor (for vector drawing, operate on current display)          */
/* -------------------------------------------------------------------------- */

void rb_display_move_to(int x, int y);
void rb_display_move_by(int x, int y);
void rb_display_draw_pixel(int x, int y);
void rb_display_draw_line(int x1, int y1, int x2, int y2);
void rb_display_draw_line_to(int x, int y);
void rb_display_draw_vstring(char *str, int x, int y);

/* -------------------------------------------------------------------------- */
/*  High-level text (operate on current display)                              */
/* -------------------------------------------------------------------------- */

boolean rb_display_print_char_at(int row, int col, char ch);
void rb_display_print_at(int row, int col, char *str);
void rb_display_print_char(char ch);
void rb_display_print(char *str);
void rb_display_print_line(char *str);
void rb_display_print_newline(void);
void rb_display_print_palette(void);
void rb_display_show_charset(void);
#endif
