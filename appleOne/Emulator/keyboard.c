#include <stdio.h>

#include "pia6820.h"
#include "keyboard.h"
#include "screen.h"

int keyboard_process_input(void) {
	char ch;

	ch = '\0';
    ch = screen_getch();
    
    if (ch == '\0')
		return 0;

    if (ch == '\n') {
		ch = '\r';
	}
    else if (ch == '\b') {
        ch = 0x5f;
    }
    else if (ch == '~') {   // Simulates esacpe character
        ch = 0x1b;
    }
    else if (ch >= 'a' && ch <= 'z') {
		ch = ch - 'a' + 'A';
	}
    
	pia6820_write_kbd((unsigned char)(ch + 0x80));
	pia6820_write_kbd_cr(0xA7); 

	return 1;
}
