#include <stdio.h>

#include "pia6820.h"
#include "keyboard.h"
#include "screen.h"

int process_keyboard_input(void) {
	char ch;

	ch = '\0';
    ch = getch_screen();
    
    if (ch == '\0')
		return 0;

    if (ch == '\n') {
		ch = '\r';
	}
    else if (ch == '\b') {
		ch = 0x5f;
	}
    else if (ch >= 'a' && ch <= 'z') {
		ch = ch - 'a' + 'A';
	}
    
	writeKbd((unsigned char)(ch + 0x80));
	writeKbdCr(0xA7); 

	return 1;
}
