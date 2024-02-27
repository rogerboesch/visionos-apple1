
#include <stdio.h>
#include "terminal.h"
#include "ret_textbuffer.h"
#include "ret_renderer.h"

char key = TERMINAL_ERR;

void terminal_print(char *str) {
    RETPrint(str);
}

void terminal_printchar(char ch) {
    RETPrintChar(ch);
}

void terminal_init(void) {
    ret_rend_initialize();
    ret_text_initialize();
    ret_text_set_immediate_mode(1);
    ret_text_cursor_show(0);
    RETSetBgColor(0);
    RETSetFgColor(13);
}

void terminal_clear(void) {
    ret_text_clear_screen();
}

void terminal_refresh(void) {
    RETRenderFrame();
}

void terminal_setcursor(int row, int col) {
    ret_text_cursor_set(col+1, row+1);
}

void terminal_setch(int ch) {
    key = ch;
}

char terminal_getch(void) {
    char current = key;
    key = TERMINAL_ERR;

    if (current != TERMINAL_ERR) {
        printf("%c\n", current);
    }
    
    return current;
}

char terminal_testch(void) {
    return key;
}

void terminal_print_state(char *str) {
    printf("STATE> %s\n", str);
}

void terminal_error(char *msg, int code) {
    printf("ERROR: %s\n", msg);
}
