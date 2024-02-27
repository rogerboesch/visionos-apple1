
#ifndef terminal_h
#define terminal_h

#define TERMINAL_ERR '\0'

void terminal_init(void);
void terminal_clear(void);
void terminal_print(char *str);
void terminal_printchar(char str);
void terminal_refresh(void);

char terminal_testch(void);
char terminal_getch(void);
void terminal_setch(int ch);

void terminal_print_state(char *str);
void terminal_error(char *msg, int code);
void terminal_setcursor(int row, int col);

#endif
