
#include <stdio.h>
#include "terminal.h"
#include "ret_textbuffer.h"
#include "ret_renderer.h"

int key = TERMINAL_ERR;

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

void terminal_setch(int ch) {
    key = ch;
}

int terminal_getch(void) {
    int current = key;
    key = TERMINAL_ERR;

    return current;
}

int terminal_testch(void) {
    return key;
}

void terminal_print_state(char *str) {
    printf("STATE> %s\n", str);
}

void terminal_error(char *msg, int code) {
    printf("ERROR: %s\n", msg);
}
