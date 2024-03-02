
#include "ret_renderer.h"
#include "ret_textbuffer.h"
#include "ret_platform_types.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#include "ret_font.h"

extern void ret_render_frame(byte* data, int index);
extern byte ret_font_apple2[];

#define RET_BUF_SIZE RET_PIXEL_WIDTH*RET_PIXEL_HEIGHT*4

typedef struct _RETScreen {
    byte bg_color, fg_color;    // Palette colors
    byte fg_brightness;         // Brightness 0-15
    byte mode_dotted;           // Dotted mode for lines (0=off, 1-255=pattern)
    boolean dirty_bit;          // Flags if screen is updated

    byte data[RET_BUF_SIZE];    // Pixel data buffer
    byte* pointer;              // Pointer to pixel data buffer

    int cursorx;                // Graphics cursor
    int cursory;
	
	int direct_render;			// 0=Needs render frame, 1=Output every character
} RETScreen;

static RETScreen ret_screen_main;
static RETScreen ret_screen_game;
static RETScreen *ret_screen_current;
static int ret_screen_index = 0;  // 1=main, 2=game
static int ret_screen_mirror = 1; // Just works together with ret_screen_index=1

// -----------------------------------------------------------------------------
#pragma mark - Vector font table

#define P(x,y) ((((x) & 0xF) << 4) | (((y) & 0xF) << 0))
#define FONT_UP 0xFE
#define FONT_LAST 0xFF

const byte ret_vecfont[128][8] = {
	['0' - 0x20] = { P(0,0), P(8,0), P(8,12), P(0,12), P(0,0), P(8,12), FONT_LAST },
	['1' - 0x20] = { P(4,0), P(4,12), P(3,10), FONT_LAST },
	['2' - 0x20] = { P(0,12), P(8,12), P(8,7), P(0,5), P(0,0), P(8,0), FONT_LAST },
	['3' - 0x20] = { P(0,12), P(8,12), P(8,0), P(0,0), FONT_UP, P(0,6), P(8,6), FONT_LAST },
	['4' - 0x20] = { P(0,12), P(0,6), P(8,6), FONT_UP, P(8,12), P(8,0), FONT_LAST },
	['5' - 0x20] = { P(0,0), P(8,0), P(8,6), P(0,7), P(0,12), P(8,12), FONT_LAST },
	['6' - 0x20] = { P(0,12), P(0,0), P(8,0), P(8,5), P(0,7), FONT_LAST },
	['7' - 0x20] = { P(0,12), P(8,12), P(8,6), P(4,0), FONT_LAST },
	['8' - 0x20] = { P(0,0), P(8,0), P(8,12), P(0,12), P(0,0), FONT_UP, P(0,6), P(8,6), },
	['9' - 0x20] = { P(8,0), P(8,12), P(0,12), P(0,7), P(8,5), FONT_LAST },
	[' ' - 0x20] = { FONT_LAST },
	['.' - 0x20] = { P(3,0), P(4,0), FONT_LAST },
	[',' - 0x20] = { P(2,0), P(4,2), FONT_LAST },
	['-' - 0x20] = { P(2,6), P(6,6), FONT_LAST },
	['+' - 0x20] = { P(1,6), P(7,6), FONT_UP, P(4,9), P(4,3), FONT_LAST },
	['!' - 0x20] = { P(4,0), P(3,2), P(5,2), P(4,0), FONT_UP, P(4,4), P(4,12), FONT_LAST },
	['#' - 0x20] = { P(0,4), P(8,4), P(6,2), P(6,10), P(8,8), P(0,8), P(2,10), P(2,2) },
	['^' - 0x20] = { P(2,6), P(4,12), P(6,6), FONT_LAST },
	['=' - 0x20] = { P(1,4), P(7,4), FONT_UP, P(1,8), P(7,8), FONT_LAST },
	['*' - 0x20] = { P(0,0), P(4,12), P(8,0), P(0,8), P(8,8), P(0,0), FONT_LAST },
	['_' - 0x20] = { P(0,0), P(8,0), FONT_LAST },
	['/' - 0x20] = { P(0,0), P(8,12), FONT_LAST },
	['\\' - 0x20] = { P(0,12), P(8,0), FONT_LAST },
	['@' - 0x20] = { P(8,4), P(4,0), P(0,4), P(0,8), P(4,12), P(8,8), P(4,4), P(3,6) },
	['$' - 0x20] = { P(6,2), P(2,6), P(6,10), FONT_UP, P(4,12), P(4,0), FONT_LAST },
	['&' - 0x20] = { P(8,0), P(4,12), P(8,8), P(0,4), P(4,0), P(8,4), FONT_LAST },
	['[' - 0x20] = { P(6,0), P(2,0), P(2,12), P(6,12), FONT_LAST },
	[']' - 0x20] = { P(2,0), P(6,0), P(6,12), P(2,12), FONT_LAST },
	['(' - 0x20] = { P(6,0), P(2,4), P(2,8), P(6,12), FONT_LAST },
	[')' - 0x20] = { P(2,0), P(6,4), P(6,8), P(2,12), FONT_LAST },
	['{' - 0x20] = { P(6,0), P(4,2), P(4,10), P(6,12), FONT_UP, P(2,6), P(4,6), FONT_LAST },
	['}' - 0x20] = { P(4,0), P(6,2), P(6,10), P(4,12), FONT_UP, P(6,6), P(8,6), FONT_LAST },
	['%' - 0x20] = { P(0,0), P(8,12), FONT_UP, P(2,10), P(2,8), FONT_UP, P(6,4), P(6,2) },
	['<' - 0x20] = { P(6,0), P(2,6), P(6,12), FONT_LAST },
	['>' - 0x20] = { P(2,0), P(6,6), P(2,12), FONT_LAST },
	['|' - 0x20] = { P(4,0), P(4,5), FONT_UP, P(4,6), P(4,12), FONT_LAST },
	[':' - 0x20] = { P(4,9), P(4,7), FONT_UP, P(4,5), P(4,3), FONT_LAST },
	[';' - 0x20] = { P(4,9), P(4,7), FONT_UP, P(4,5), P(1,2), FONT_LAST },
	['"' - 0x20] = { P(2,10), P(2,6), FONT_UP, P(6,10), P(6,6), FONT_LAST },
	['\'' - 0x20] = { P(2,6), P(6,10), FONT_LAST },
	['`' - 0x20] = { P(2,10), P(6,6), FONT_LAST },
	['~' - 0x20] = { P(0,4), P(2,8), P(6,4), P(8,8), FONT_LAST },
	['?' - 0x20] = { P(0,8), P(4,12), P(8,8), P(4,4), FONT_UP, P(4,1), P(4,0), FONT_LAST },
	['A' - 0x20] = { P(0,0), P(0,8), P(4,12), P(8,8), P(8,0), FONT_UP, P(0,4), P(8,4) },
	['B' - 0x20] = { P(0,0), P(0,12), P(4,12), P(8,10), P(4,6), P(8,2), P(4,0), P(0,0) },
	['C' - 0x20] = { P(8,0), P(0,0), P(0,12), P(8,12), FONT_LAST },
	['D' - 0x20] = { P(0,0), P(0,12), P(4,12), P(8,8), P(8,4), P(4,0), P(0,0), FONT_LAST },
	['E' - 0x20] = { P(8,0), P(0,0), P(0,12), P(8,12), FONT_UP, P(0,6), P(6,6), FONT_LAST },
	['F' - 0x20] = { P(0,0), P(0,12), P(8,12), FONT_UP, P(0,6), P(6,6), FONT_LAST },
	['G' - 0x20] = { P(6,6), P(8,4), P(8,0), P(0,0), P(0,12), P(8,12), FONT_LAST },
	['H' - 0x20] = { P(0,0), P(0,12), FONT_UP, P(0,6), P(8,6), FONT_UP, P(8,12), P(8,0) },
	['I' - 0x20] = { P(0,0), P(8,0), FONT_UP, P(4,0), P(4,12), FONT_UP, P(0,12), P(8,12) },
	['J' - 0x20] = { P(0,4), P(4,0), P(8,0), P(8,12), FONT_LAST },
	['K' - 0x20] = { P(0,0), P(0,12), FONT_UP, P(8,12), P(0,6), P(6,0), FONT_LAST },
	['L' - 0x20] = { P(8,0), P(0,0), P(0,12), FONT_LAST },
	['M' - 0x20] = { P(0,0), P(0,12), P(4,8), P(8,12), P(8,0), FONT_LAST },
	['N' - 0x20] = { P(0,0), P(0,12), P(8,0), P(8,12), FONT_LAST },
	['O' - 0x20] = { P(0,0), P(0,12), P(8,12), P(8,0), P(0,0), FONT_LAST },
	['P' - 0x20] = { P(0,0), P(0,12), P(8,12), P(8,6), P(0,5), FONT_LAST },
	['Q' - 0x20] = { P(0,0), P(0,12), P(8,12), P(8,4), P(0,0), FONT_UP, P(4,4), P(8,0) },
	['R' - 0x20] = { P(0,0), P(0,12), P(8,12), P(8,6), P(0,5), FONT_UP, P(4,5), P(8,0) },
	['S' - 0x20] = { P(0,2), P(2,0), P(8,0), P(8,5), P(0,7), P(0,12), P(6,12), P(8,10) },
	['T' - 0x20] = { P(0,12), P(8,12), FONT_UP, P(4,12), P(4,0), FONT_LAST },
	['U' - 0x20] = { P(0,12), P(0,2), P(4,0), P(8,2), P(8,12), FONT_LAST },
	['V' - 0x20] = { P(0,12), P(4,0), P(8,12), FONT_LAST },
	['W' - 0x20] = { P(0,12), P(2,0), P(4,4), P(6,0), P(8,12), FONT_LAST },
	['X' - 0x20] = { P(0,0), P(8,12), FONT_UP, P(0,12), P(8,0), FONT_LAST },
	['Y' - 0x20] = { P(0,12), P(4,6), P(8,12), FONT_UP, P(4,6), P(4,0), FONT_LAST },
	['Z' - 0x20] = { P(0,12), P(8,12), P(0,0), P(8,0), FONT_UP, P(2,6), P(6,6), FONT_LAST },

	// Special chars from 65 to 96
	[65] = { P(0,0), P(0,12), P(7,12), P(7,0), P(0,0), FONT_LAST },
	[66] = { P(0,0), P(4,12), P(8,0), P(0,0), FONT_LAST },
	[67] = { P(0,6), P(8,6), P(8,7), P(0,7), FONT_LAST },
};

// -----------------------------------------------------------------------------
#pragma mark - Helper

int ret_getbit(byte b, byte number) {
    // TODO: Make flexible when font must be inverted, Apple 2 needs it
    number = 8 - number;
    return (b >> number) & 1;
}

// -----------------------------------------------------------------------------
#pragma mark - Dirty bit mechanism

byte ret_rend_get_dirty_bit(void) {
    return ret_screen_current->dirty_bit;
}

void ret_rend_enable_dirty_bit(void) {
    ret_screen_current->dirty_bit = 1;
}

void ret_rend_disable_dirty_bit(void) {
    ret_screen_current->dirty_bit = 0;
}

// -----------------------------------------------------------------------------
#pragma mark - Pixel drawing

void ret_rend_set_pixel_internal(int x, int y, int r, int g, int b) {
    int width = RET_PIXEL_WIDTH;
    int height = RET_PIXEL_HEIGHT;

    // Is the pixel actually visible?
    if (x >= 0 && x < width && y >= 0 && y < height) {
        int offset = (x + ((height - 1) - y) * width) * 4;
        
        // Use this to make the origin top-left instead of bottom-right.
        offset = (x + y * width) * 4;
        
        ret_screen_current->data[offset] = r;
        ret_screen_current->data[offset+1] = g;
        ret_screen_current->data[offset+2] = b;
        ret_screen_current->data[offset+3] = 255;
    }

    ret_rend_enable_dirty_bit();
}

void ret_rend_set_pixel(int x, int y, byte paletteColor, byte brightness) {
    ret_color fg = RETPaletteGetColorWithBrightness(paletteColor, brightness);
    ret_rend_set_pixel_internal(x, y, fg.r, fg.g, fg.b);
}

void ret_rend_draw_line(int x1, int y1, int x2, int y2, byte paletteColor, byte brightness)  {
    int dx = x2 - x1;
    int dy = y2 - y1;

    // calculate steps required for generating pixels
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    
    // calculate increment in x & y for each steps
    float xInc = dx / (float) steps;
    float yInc = dy / (float) steps;
    
    float x = x1;
    float y = y1;

	boolean draw = true;
	int count = 0;
	
    for (int i = 0; i <= steps; i++) {
		if (ret_screen_current->mode_dotted > 0) {
			if (draw) {
				ret_rend_set_pixel (x, y, paletteColor, brightness);
			}
			
			count++;

			if (count == ret_screen_current->mode_dotted) {
				count = 0;
				draw = !draw;
			}
		}
		else {
			ret_rend_set_pixel (x, y, paletteColor, brightness);
		}
		
        x += xInc;
        y += yInc;
    }
}

// -----------------------------------------------------------------------------
#pragma mark - Character vector drawing (internal)

int ret_rend_cursorx_int = 0;
int ret_rend_cursory_int = 0;

void ret_rend_moveto(int x, int y) {
    ret_rend_cursorx_int = x;
    ret_rend_cursory_int = y;
}

void ret_rend_moveby(int dx, int dy) {
    ret_rend_cursorx_int += dx;
    ret_rend_cursory_int += dy;
}

void ret_rend_lineby(int dx, int dy) {
    ret_rend_draw_line(ret_rend_cursorx_int, ret_rend_cursory_int, ret_rend_cursorx_int+dx, ret_rend_cursory_int+dy, ret_screen_current->fg_color, ret_screen_current->fg_brightness);
    ret_rend_moveby(dx, dy);
}

void ret_rend_vec_drawchar(char ch) {
    const byte* p = ret_vecfont[ch-0x20];
    byte bright = 0;
    byte x = 0;
    byte y = 0;
    byte i;

    for (i=0; i<8; i++) {
        byte b = *p++;
        
        if (b == FONT_LAST)
            break; // last move
        else if (b == FONT_UP)
            bright = 0; // pen up
        else {
            byte x2 = b>>4;
            byte y2 = b&15;

            if (bright == 0)
                ret_rend_moveby((char)(x2-x), (char)-(y2-y));
            else
                ret_rend_lineby((char)(x2-x), (char)-(y2-y));

            bright = 4;
            x = x2;
            y = y2;
        }
    }
}

// -----------------------------------------------------------------------------
#pragma mark - Text drawing

void ret_rend_draw_char(int x, int y, const char ch, int invert, byte paletteColor) {
    int w = RET_FONT_WIDTH;
    int h = RET_FONT_HEIGHT;
    int offset = (int)ch * 8;

    byte* dest = ret_screen_current->pointer + y * (RET_PIXEL_WIDTH*4) + (x*4);
    
    ret_color bg = RETPaletteGetColor(ret_screen_current->bg_color);
    ret_color fg = RETPaletteGetColorWithBrightness(paletteColor, ret_screen_current->fg_brightness);

	int xStart = x;
    for (int iy = 0; iy < h; iy++) {
        byte line = ret_font_apple2[offset+iy];

        for (int ix = w-1; ix >= 0; ix--) {
			if (x >= 0 && x < RET_PIXEL_WIDTH && y >= 0 && y < RET_PIXEL_HEIGHT) {
                if (ch == ' ' && invert) {
                    // Draw pixel
                    *dest = fg.r; dest++;
                    *dest = fg.g; dest++;
                    *dest = fg.b; dest++;
                    *dest = 0xff; dest++;
                }
				else if (ret_getbit(line, ix) == !invert) {
					// Draw pixel
					*dest = fg.r; dest++;
					*dest = fg.g; dest++;
					*dest = fg.b; dest++;
					*dest = 0xff; dest++;
				}
				else {
					// Erase pixel
					*dest = bg.r; dest++;
					*dest = bg.g; dest++;
					*dest = bg.b; dest++;
					*dest = 0xff; dest++;
				}
			}
			
			// Next pixel
			x++;
        }
        
        // Next line
        y++;
		x = xStart;
		
        dest = ret_screen_current->pointer + y * (RET_PIXEL_WIDTH*4) + (x*4);
    }
    
    ret_rend_enable_dirty_bit();
	
	if (ret_screen_current->direct_render) {
		RETRenderFrame();
	}
}

// -----------------------------------------------------------------------------
#pragma mark - Pixel buffer rendering

void ret_rend_clear_screen() {
    int offset = 0;

    ret_color color = RETPaletteGetColor(ret_screen_current->bg_color);

    for (int i = 0 ; i < RET_PIXEL_WIDTH * RET_PIXEL_HEIGHT ; ++i) {
        ret_screen_current->data[offset] = color.r;
        ret_screen_current->data[offset+1] = color.g;
        ret_screen_current->data[offset+2] = color.b;
        ret_screen_current->data[offset+3] = 255;
        
        offset += 4;
    }
    
    ret_rend_enable_dirty_bit();
}

void ret_rend_scroll_up(int height) {
    byte* source;
    byte* dest;

    for (int y = height; y < RET_PIXEL_HEIGHT; y++) {
        source = &ret_screen_current->data[y*RET_PIXEL_WIDTH*4];
        dest = &ret_screen_current->data[(y-height)*RET_PIXEL_WIDTH*4];
        
        memcpy(dest, source, RET_PIXEL_WIDTH*4);
    }

    ret_color color = RETPaletteGetColor(ret_screen_current->bg_color);

    // clear last lines
    int offset = (RET_PIXEL_HEIGHT-height) * RET_PIXEL_WIDTH*4;

    for (int i = 0; i < height*RET_PIXEL_WIDTH; ++i) {
        ret_screen_current->data[offset] = color.r;
        ret_screen_current->data[offset+1] = color.g;
        ret_screen_current->data[offset+2] = color.b;
        ret_screen_current->data[offset+3] = 255;
        
        offset += 4;
    }

    ret_rend_enable_dirty_bit();
}

// -----------------------------------------------------------------------------
#pragma mark - Private color functions

ret_color ret_rend_get_bg_color() {
	return RETPaletteGetColor(ret_screen_current->bg_color);
}

// -----------------------------------------------------------------------------
#pragma mark - Character vector drawing (public)

void RETDrawVString(char* str, int x, int y) {
    int x2 = x;
    
    while (*str != 0) {
        char ch = *str;
    
        ret_rend_moveto(x2, y+RET_FONT_HEIGHT);
        ret_rend_vec_drawchar(ch);
        
        str++;
        x2 += RET_FONT_WIDTH+RET_VFONT_SPACE;
    }
}

// -----------------------------------------------------------------------------
#pragma mark - Public color functions

byte RETSetBgColor(byte index) {
    byte color = ret_screen_current->bg_color;
    ret_screen_current->bg_color = index;
    
    return color;
}

byte RETGetBgColor() {
	return ret_screen_current->bg_color;
}

byte RETSetFgColor(byte index) {
    byte color = ret_screen_current->fg_color;
    ret_screen_current->fg_color = index;
    
    return color;
}

byte RETGetFgColor() {
	return ret_screen_current->fg_color;
}

byte RETSetFgBrightness(byte index) {
    byte brightness = ret_screen_current->fg_brightness;
	ret_screen_current->fg_brightness = index;
    
    return brightness;
}

byte RETGetFgBrightness() {
	return ret_screen_current->fg_brightness;
}

byte RETSetDotMode(byte mode) {
    byte old = ret_screen_current->mode_dotted;
	ret_screen_current->mode_dotted = mode;
    
    return old;
}

// -----------------------------------------------------------------------------
#pragma mark - Public drawing functions

void RETDrawPixel(int x, int y) {
    // 0,0 is at center of screen
    ret_rend_set_pixel(RET_PIXEL_WIDTH_2+x, RET_PIXEL_HEIGHT_2+y, ret_screen_current->fg_color, ret_screen_current->fg_brightness);

    ret_screen_current->cursorx = RET_PIXEL_WIDTH_2+x;
    ret_screen_current->cursory = RET_PIXEL_HEIGHT_2+y;
}

void RETDrawLine(int x1, int y1, int x2, int y2) {
    ret_rend_draw_line(RET_PIXEL_WIDTH_2+x1, RET_PIXEL_HEIGHT_2+y1, RET_PIXEL_WIDTH_2+x2, RET_PIXEL_HEIGHT_2+y2, ret_screen_current->fg_color, ret_screen_current->fg_brightness);

    ret_screen_current->cursorx = RET_PIXEL_WIDTH_2+x2;
    ret_screen_current->cursory = RET_PIXEL_HEIGHT_2+y2;
}

void RETDrawLineTo(int x, int y) {
    ret_rend_draw_line(ret_screen_current->cursorx, ret_screen_current->cursory, ret_screen_current->cursorx+x, ret_screen_current->cursory+y, ret_screen_current->fg_color, ret_screen_current->fg_brightness);
    
    ret_screen_current->cursorx = ret_screen_current->cursorx+x;
    ret_screen_current->cursory = ret_screen_current->cursory+y;
}

void RETMoveTo(int x, int y) {
    ret_screen_current->cursorx = RET_PIXEL_WIDTH_2+x;
    ret_screen_current->cursory = RET_PIXEL_HEIGHT_2+y;
}

void RETMoveBy(int x, int y) {
    ret_screen_current->cursorx += x;
    ret_screen_current->cursory += y;
}

void RETRenderFrame(void) {
    if (ret_screen_mirror == 1) {
        ret_render_frame(ret_screen_current->pointer, 0);
    }
    else {
        ret_render_frame(ret_screen_current->pointer, ret_screen_index);
    }
}

void RETSetMainScreen(void) {
    ret_screen_current = &ret_screen_main;
    ret_screen_index = 1;
}

void RETSetScreenMirror(int mode) {
    RETSetMainScreen();
    ret_screen_mirror = mode;
}

void RETSetGameScreen(void) {
    if (ret_screen_mirror == 1) {
        ret_screen_mirror = 0;
    }
    
    ret_screen_current = &ret_screen_game;
    ret_screen_index = 2;
}

void RETSetDirectRender(int mode) {
	ret_screen_current->direct_render = mode;
}

// -----------------------------------------------------------------------------
#pragma mark - Pixel buffer initialize

void ret_rend_initialize() {
    ret_palette_create_atari();
    
    ret_screen_main.bg_color = RET_COLOR_BG;
    ret_screen_main.fg_color = RET_COLOR_FG;
    ret_screen_main.fg_brightness = RET_BRIGHTNESS_NORMAL;
    ret_screen_main.mode_dotted = 0;
    ret_screen_main.dirty_bit = 0;
    ret_screen_main.cursorx = 0;
    ret_screen_main.cursory = 0;
    ret_screen_main.pointer = &ret_screen_main.data[0];
	ret_screen_main.direct_render = 0;
	
    ret_screen_game.bg_color = RET_COLOR_BG;
    ret_screen_game.fg_color = RET_COLOR_FG;
    ret_screen_game.fg_brightness = RET_BRIGHTNESS_NORMAL;
    ret_screen_game.mode_dotted = 0;
    ret_screen_game.dirty_bit = 0;
    ret_screen_game.cursorx = 0;
    ret_screen_game.cursory = 0;
    ret_screen_game.pointer = &ret_screen_game.data[0];
	ret_screen_game.direct_render = 0;

    RETSetMainScreen();
    RETMoveTo(0,0);
}
