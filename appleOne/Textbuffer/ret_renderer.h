#ifndef RET_RENDERER_H
#define RET_RENDERER_H

#include "ret_palette.h"
#include "ret_font.h"

#define RET_PIXEL_WIDTH_APPLE_I 336
#define RET_PIXEL_HEIGHT_APPLE_I 208
#define RET_PIXEL_WIDTH_RET 640
#define RET_PIXEL_HEIGHT_RET 480
#define RET_PIXEL_WIDTH_C64 320
#define RET_PIXEL_HEIGHT_C64 200
#define RET_PIXEL_WIDTH_ATARIXL 320
#define RET_PIXEL_HEIGHT_ATARIXL 192
#define RET_PIXEL_WIDTH_VECTREX 440
#define RET_PIXEL_HEIGHT_VECTREX 546
#define RET_PIXEL_WIDTH_TEST 480
#define RET_PIXEL_HEIGHT_TEST 320

#define RET_PIXEL_WIDTH RET_PIXEL_WIDTH_APPLE_I
#define RET_PIXEL_HEIGHT RET_PIXEL_HEIGHT_APPLE_I

#define RET_PIXEL_WIDTH_2 RET_PIXEL_WIDTH/2
#define RET_PIXEL_HEIGHT_2 RET_PIXEL_HEIGHT/2

#define RET_TEXT_WIDTH RET_PIXEL_WIDTH/RET_FONT_WIDTH
#define RET_TEXT_HEIGHT RET_PIXEL_HEIGHT/RET_FONT_HEIGHT

void ret_rend_set_pixel(int x, int y, byte paletteColor, byte brightness);
void ret_rend_clear_screen(void);

void ret_rend_draw_char(int x, int y, const char ch, int invert, byte paletteColor);
void ret_rend_scroll_up(int height);

void ret_rend_initialize(void);
ret_color ret_rend_get_bg_color(void);

byte RETSetBgColor(byte index);
byte RETGetBgColor(void);
byte RETSetFgColor(byte index);
byte RETGetFgColor(void);
byte RETSetFgBrightness(byte index);
byte RETGetFgBrightness(void);
byte RETSetDotMode(byte mode);

void RETMoveTo(int x, int y);
void RETMoveBy(int x, int y);
void RETDrawPixel(int x, int y);
void RETDrawLine(int x1, int y1, int x2, int y2);
void RETDrawLineTo(int x1, int y1);
void RETRenderFrame(void);

void RETSetMainScreen(void);
void RETSetGameScreen(void);
void RETSetDirectRender(int);
void RETSetScreenMirror(int mode);

void RETDrawVString(char* str, int x, int y);

#endif
