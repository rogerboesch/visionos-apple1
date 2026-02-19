
#include <stdio.h>
#include "terminal.h"
#include "rb_display.h"

#define TERMINAL_DISPLAY 0

char key = TERMINAL_ERR;

void terminal_print(char *str) {
    rb_display_print(TERMINAL_DISPLAY, str);
}

void terminal_printchar(char ch) {
    rb_display_print_char(TERMINAL_DISPLAY, ch);
}

void terminal_init(void) {
    rb_display_init(42, 26);
    rb_display_text_set_immediate(TERMINAL_DISPLAY, 1);
    rb_display_cursor_show(TERMINAL_DISPLAY, 0);
    rb_display_set_bg_color(TERMINAL_DISPLAY, 0);
    rb_display_set_fg_color(TERMINAL_DISPLAY, 13);
}

void terminal_clear(void) {
    rb_display_text_clear(TERMINAL_DISPLAY);
}

void terminal_refresh(void) {
    rb_display_render_frame(TERMINAL_DISPLAY);
}

void terminal_setcursor(int row, int col) {
    rb_display_cursor_set(TERMINAL_DISPLAY, col + 1, row + 1);
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
