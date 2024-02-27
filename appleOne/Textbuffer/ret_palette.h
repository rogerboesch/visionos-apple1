
#ifndef ret_palette_h
#define ret_palette_h

#define RET_PALETTE_SIZE 16
#define RET_BRIGHTNESS_NORMAL 7

#define RET_COLOR_BLACK         0
#define RET_COLOR_WHITE         1
#define RET_COLOR_RED           2
#define RET_COLOR_TURKEY        3
#define RET_COLOR_VIOLETT       4
#define RET_COLOR_GREEN         5
#define RET_COLOR_BLUE          6
#define RET_COLOR_YELLOW        7
#define RET_COLOR_ORANGE        8
#define RET_COLOR_BROWN         9
#define RET_COLOR_REDLIGHT      10
#define RET_COLOR_GRAY1         11
#define RET_COLOR_GRAY2         12
#define RET_COLOR_GREENLIGHT    13
#define RET_COLOR_BLUELIGHT     14
#define RET_COLOR_GRAY3         15

typedef unsigned char byte;

typedef struct ret_color_ {
	byte r, g, b;
} ret_color;

void ret_palette_create_atari(void);
ret_color RETPaletteGetColor(byte index);
ret_color RETPaletteGetColorWithBrightness(byte index, byte bindex);

#endif
