#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "screen.h"
#include "statusbar.h"

void statusbar_print(char *s) {
    printf("STATUS PRINT: %s\n", s);
    // Not use yet, use to build a status bar
	// char msg[MSG_LEN_MAX + 1];
    // printf("%s", msg);
}

void statusbar_init(void) {
    // msgbuf = newwin(1, ncol, nrow, 0);
    // wattron(msgbuf, COLOR_PAIR(1) | A_REVERSE); /* 2 is black on green */
	// print_msgbuf("");
}

void statusbar_input(char *prompt, char *typed) {
    printf("STATUS INPUT: %s\n", prompt);
    /*
	werase(msgbuf);
	echo();
	nocbreak();
	wprintw(msgbuf, "%s", prompt);
	wrefresh(msgbuf);
	wgetnstr(msgbuf, typed, MSG_LEN_MAX);
	noecho();
	cbreak();
	print_msgbuf("");
	select_screen();
     */
}
	
