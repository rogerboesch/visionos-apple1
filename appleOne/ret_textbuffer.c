
#include "ret_platform_types.h"
#include "ret_renderer.h"
#include "ret_textbuffer.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

byte ret_text_screen[RET_TEXT_WIDTH*RET_TEXT_HEIGHT];		// text buffer
byte* ret_text_buffer;										// pointer to text buffer
byte ret_text_color[RET_TEXT_WIDTH*RET_TEXT_HEIGHT];		// text color buffer
byte* ret_text_colbuf;										// pointer to text color buffer
int ret_text_immediate_print = 1;							// 0=needs a flush to print
boolean ret_text_cursor_visible = true;						// Cursor visible
boolean ret_invert_mode = false;                            // Draw characer inverted

int ret_cursor_row = 0;											// Cursor position
int ret_cursor_col = 0;
int cursor_blink = 0;										// Cursor blink state, 0=Off
int ret_cursor_cycles = 0;										// Cursor cycle count
char ret_line_buffer[RET_TEXT_WIDTH];							// Text from a single line after press return

// -----------------------------------------------------------------------------
#pragma mark - Text buffer rendering

void ret_text_set_immediate_mode(int flag) {
	// 0=text buffering until flush, 1=direct print
	ret_text_immediate_print = flag;
}

void ret_text_flush_buffer() {
	for (int row = 0; row < RET_TEXT_HEIGHT; row++) {
		for (int col = 0; col < RET_TEXT_WIDTH; col++) {
			char ch = ret_text_buffer[row*RET_TEXT_WIDTH+col];
			byte paletteColor = ret_text_colbuf[row*RET_TEXT_WIDTH+col];

			int x = col * RET_FONT_WIDTH;
			int y = row * RET_FONT_HEIGHT;

			ret_rend_draw_char(x, y, ch, 0, paletteColor);
		}
	}
}

void ret_text_fast_clear_screen() {
	memset(ret_text_buffer, 0, RET_TEXT_WIDTH*RET_TEXT_HEIGHT);
}

void ret_text_clear_screen() {
	ret_cursor_row = 0;
	ret_cursor_col = 0;

	ret_text_fast_clear_screen();
    ret_text_flush_buffer();
}

boolean ret_text_print_character(int row, int col, char ch, byte color) {
    int width = RET_TEXT_WIDTH;
    int height = RET_TEXT_HEIGHT;
        
    byte code = (byte)ch;
    if (code >= RET_ESCAPE_BASE_INDEX_FOR_COLOR && code <= RET_ESCAPE_BASE_INDEX_FOR_COLOR + 15) {
        // Color escape
        int color = (int)code - RET_ESCAPE_BASE_INDEX_FOR_COLOR;
        RETSetFgColor(color);
        
        return false;
    }
    
    if (code == RET_ESCAPE_INVERT_ON) {
        RETSetInvertMode(true);
        
        return false;
    }

    if (code == RET_ESCAPE_INVERT_OFF) {
        RETSetInvertMode(false);
        
        return false;
    }

    // Is the pixel actually visible?
    if (col >= 0 && col < width && row >= 0 && row < height) {
        int offset = row * RET_TEXT_WIDTH + col;
        
        ret_text_screen[offset] = ch;
        ret_text_color[offset] = color;
        
        if (ret_text_immediate_print == 1) {
            int x = col * RET_FONT_WIDTH;
            int y = row * RET_FONT_HEIGHT;

            if (ret_invert_mode) {
                ret_rend_draw_char(x, y, ch, 1, color);
            }
            else {
                ret_rend_draw_char(x, y, ch, 0, color);
            }
        }
    }
	
	return true;
}

char ret_text_get_character(int row, int col) {
    int offset = row * RET_TEXT_WIDTH + col;
    return ret_text_screen[offset];
}

boolean ret_text_print_character_internal(int row, int col, char ch) {
    return ret_text_print_character(row, col, ch, RETGetFgColor());
}

void ret_text_scroll_up() {
	byte* source;
	byte* dest;
	
	for (int row = 1; row < RET_TEXT_HEIGHT; row++) {
		source = &ret_text_screen[row*RET_TEXT_WIDTH];
		dest = &ret_text_screen[(row-1)*RET_TEXT_WIDTH];
		
		memcpy(dest, source, RET_TEXT_WIDTH);
	}

	ret_rend_scroll_up(RET_FONT_HEIGHT);

	// Clear last line
	dest = &ret_text_screen[(RET_TEXT_HEIGHT-1)*RET_TEXT_WIDTH];
	memset(dest, '\0', RET_TEXT_WIDTH);
}


// -----------------------------------------------------------------------------
#pragma mark - Cursor handling

void ret_text_cursor_show(boolean flag) {
    ret_text_cursor_visible = flag;
}

void ret_text_draw_cursor() {
    if (!ret_text_cursor_visible) {
        return;
    }
    
	int x = ret_cursor_col * RET_FONT_WIDTH;
	int y = ret_cursor_row * RET_FONT_HEIGHT;

    char ch = ret_text_screen[ret_cursor_row*RET_TEXT_WIDTH+ret_cursor_col];

    ret_rend_draw_char(x, y, ch, 1, RET_COLOR_CURSOR);
}

void ret_text_reset_character() {
	int x = ret_cursor_col * RET_FONT_WIDTH;
	int y = ret_cursor_row * RET_FONT_HEIGHT;

    char ch = ret_text_screen[ret_cursor_row*RET_TEXT_WIDTH+ret_cursor_col];
    byte paletteColor = ret_text_colbuf[ret_cursor_row*RET_TEXT_WIDTH+ret_cursor_col];

    ret_rend_draw_char(x, y, ch, 0, paletteColor);
}

void ret_text_cursor_left() {
	ret_text_reset_character();
	
	ret_cursor_col--;
	
	if (ret_cursor_col < 0) {
		ret_cursor_row--;
		ret_cursor_col = RET_TEXT_WIDTH-1;
	}
	
	if (ret_cursor_row < 0) {
		ret_cursor_row = 0;
		ret_cursor_col = 0;
	}

	ret_text_draw_cursor();
}

void ret_text_cursor_right() {
	ret_text_reset_character();

	ret_cursor_col++;
	
	if (ret_cursor_col > RET_TEXT_WIDTH-1) {
		ret_cursor_row++;
		ret_cursor_col = 0;
	}

	if (ret_cursor_row > RET_TEXT_HEIGHT-1) {
		ret_text_scroll_up();
		
		ret_cursor_row = RET_TEXT_HEIGHT-1;
	}
	
	ret_text_draw_cursor();
}

void ret_text_cursor_up() {
	ret_text_reset_character();

	ret_cursor_row--;
		
	if (ret_cursor_row < 0) {
		ret_cursor_row = 0;
	}

	ret_text_draw_cursor();
}

void ret_text_cursor_down() {
	ret_text_reset_character();

	ret_cursor_row++;
	
	if (ret_cursor_row > RET_TEXT_HEIGHT-1) {
		ret_text_scroll_up();

		ret_cursor_row = RET_TEXT_HEIGHT-1;
	}
	
	ret_text_draw_cursor();
}

void ret_text_cursor_set(int x, int y) {
    ret_text_reset_character();

    ret_cursor_col = x;
    ret_cursor_row = y;
    
    ret_text_draw_cursor();
}

void ret_text_new_line() {
	ret_text_reset_character();

	ret_cursor_row++;
	ret_cursor_col = 0;
	
	if (ret_cursor_row >= RET_TEXT_HEIGHT) {
		// Scroll screen up
		ret_text_scroll_up();
		
		ret_cursor_row = RET_TEXT_HEIGHT-1;
	}
	
	ret_text_draw_cursor();
}

void ret_text_blink_cursor() {
	ret_cursor_cycles++;
	if (ret_cursor_cycles < 25)
		return;
	
	ret_cursor_cycles = 0;
	
	if (cursor_blink == 0) {
		ret_text_draw_cursor();
	}
	else {
		ret_text_reset_character();
	}
	
	cursor_blink = !cursor_blink;
}

void ret_text_delete_char(void) {
	ret_text_cursor_left();
	
	// Remove character
	ret_text_screen[ret_cursor_row*RET_TEXT_WIDTH+ret_cursor_col] = '\0';
}

// -----------------------------------------------------------------------------
#pragma mark - Private text functions

void ret_text_initialize() {
	ret_cursor_row = 0;
	ret_cursor_col = 0;
	cursor_blink = 0;
	ret_cursor_cycles = 0;

	ret_text_buffer = &ret_text_screen[0];
	ret_text_colbuf = &ret_text_color[0];
	ret_text_fast_clear_screen();
	
	ret_text_draw_cursor();
}

void ret_text_print_no_line_break(char* str, int size) {
    int count = 0;
    while (*str != 0) {
        char ch = *str;
    
        RETPrintCharAt(ret_cursor_row, ret_cursor_col, ch);
        
        ret_cursor_col++;
        if (ret_cursor_col >= RET_TEXT_WIDTH) {
            ret_cursor_col = RET_TEXT_WIDTH-1;
        }
        
        ret_text_draw_cursor();

        str++;
        count++;
        
        if (count == size) {
            return;
        }
    }
}

const char* ret_text_get_line_buffer(void) {
    return &ret_line_buffer[0];
    
}

// -----------------------------------------------------------------------------
#pragma mark - Public text functions

boolean RETPrintCharAt(int row, int col, char ch) {
	return ret_text_print_character_internal(row, col, ch);
}

void RETPrintAt(int row, int col, char* str) {
	while (*str != 0) {
		char ch = *str;
	
		RETPrintCharAt(row, col, ch);
		col++;
		
		str++;
	}
}

void RETPrintChar(char ch) {
    if (ch == '\n') {
        ret_text_new_line();
        return;
    }
    
	if (!RETPrintCharAt(ret_cursor_row, ret_cursor_col, ch)) {
		return;
	}
	
	ret_cursor_col++;
	if (ret_cursor_col >= RET_TEXT_WIDTH) {
		ret_cursor_row++;
		ret_cursor_col = 0;
	}

    if (ret_cursor_row >= RET_TEXT_HEIGHT) {
        // Scroll screen up
        ret_text_scroll_up();
        
        ret_cursor_row = RET_TEXT_HEIGHT-1;
    }

	ret_text_draw_cursor();
}

void RETPrint(char* str) {
	while (*str != 0) {
		char ch = *str;
	
		RETPrintChar(ch);
		
		str++;
	}
}

void RETPrintLine(char* str) {
	RETPrint(str);	
	ret_text_new_line();
}

void RETPrintNewLine() {
    ret_text_new_line();
}

void RETPrintPalette() {
	int fg = RETGetFgColor();

	// Print characters
	for (int i = 0; i < 128; i++) {
		RETSetFgColor(i);
		RETPrintChar((char)233);
	}
	
	RETPrintLine("");
	
	RETSetFgColor(fg);
}

void RETShowCharacterSet() {
    for (int i = 0; i < 256; i++) {
        RETPrintChar(i);
    }
}

void RETSetInvertMode(boolean flag) {
    ret_invert_mode = flag;
}

// -----------------------------------------------------------------------------
#pragma mark - Screen editor support

void RETProcessAsciiKey(int ch) {
	// Check for printable ascii codes
	if (ch == RET_KEY_ENTER) {
		// Get current line
		memset(&ret_line_buffer, ' ', RET_TEXT_WIDTH);
		
		for (int x = 0; x < ret_cursor_col; x++) {
			if (ret_text_screen[ret_cursor_row*RET_TEXT_WIDTH+x] != '\0') {
				ret_line_buffer[x] = ret_text_screen[ret_cursor_row*RET_TEXT_WIDTH+x];
			}
		}
		
		ret_line_buffer[ret_cursor_col] = '\0';

		ret_text_new_line();
	}
    else if (ch == RET_KEY_BACKSPACE) {
        // Backspace
        ret_text_delete_char();
    }
    else if (ch == RET_KEY_ARROW_LEFT) {
        ret_text_cursor_left();
    }
    else if (ch == RET_KEY_ARROW_RIGHT) {
        ret_text_cursor_right();
    }
    else if (ch == RET_KEY_ARROW_UP) {
        ret_text_cursor_up();
    }
    else if (ch == RET_KEY_ARROW_DOWN) {
        ret_text_cursor_down();
    }
	else if (ch >= 32) {
        if (ch >= 'a' && ch <= 'z') {
            ch += 'A' - 'a';
        }

		RETPrintChar(ch);
	}
}
