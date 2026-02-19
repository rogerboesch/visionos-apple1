
#include "portraits.h"
#include "ret_renderer.h"
#include "ret_textbuffer.h"

#include <string.h>

// Screen grid: 42 columns x 26 rows
#define PORTRAIT_COLS 42
#define PORTRAIT_ROWS 26

// Steve Jobs ASCII art portrait (22 rows x 30 cols)
#define JOBS_ART_ROWS 22
static const char *jobs_art[JOBS_ART_ROWS] = {
    "          ......          ",
    "       ..::::::::..       ",
    "     .:::::::::::::::     ",
    "    ::::    :::    ::::   ",
    "   :::  @@  :::  @@  ::  ",
    "   ::   @@  :::  @@  ::  ",
    "   ::       :::       :: ",
    "   :::      :::      ::: ",
    "    ::      ...      ::  ",
    "    :::   .......   :::  ",
    "     ::::.........::::   ",
    "      ::::::::::::::     ",
    "     :::::::  :::::::    ",
    "    ::::::::  ::::::::   ",
    "   :::::::::::::::::::   ",
    "   :::::::::::::::::::   ",
    "  ::::::::::::::::::::   ",
    "  ::::::::::::::::::::   ",
    "  ::::::::::::::::::::   ",
    "  ::::::::::::::::::::   ",
    "   ::::::::::::::::::    ",
    "    ::::::::::::::::     "
};

// Steve Wozniak ASCII art portrait (22 rows x 30 cols)
#define WOZ_ART_ROWS 22
static const char *woz_art[WOZ_ART_ROWS] = {
    "        ............      ",
    "     ..................   ",
    "    ..::::::::::::::::..  ",
    "   .:::::::::::::::::::.  ",
    "   :::::::::::::::::::::: ",
    "  ::: @@@ ::::: @@@ :::  ",
    "  ::: @@@ ::::: @@@ :::  ",
    "  :::::::::.:.::::::::::  ",
    "  :::::::::::::::::::::.  ",
    "   :::::  .::::.  :::::  ",
    "   ::::::........::::::  ",
    "    ::::::::::::::::::::  ",
    "    .:::::::::::::::::::  ",
    "   ...##################  ",
    "  ######################  ",
    "  ######################  ",
    "  ######################  ",
    "   ####################   ",
    "    ##################    ",
    "      ##############      ",
    "     ::::::::::::::::     ",
    "    ::::::::::::::::::    "
};

// Draw a portrait centered on screen with name below
static void draw_portrait(const char **art, int art_rows, const char *name) {
    ret_rend_clear_screen();

    byte old_fg = RETSetFgColor(RET_COLOR_GREEN);
    byte old_bright = RETSetFgBrightness(15);

    // Center the art vertically (leave room for name at bottom)
    int start_row = (PORTRAIT_ROWS - art_rows - 2) / 2;
    if (start_row < 0) start_row = 0;

    for (int row = 0; row < art_rows; row++) {
        const char *line = art[row];
        int len = (int)strlen(line);
        int col_offset = (PORTRAIT_COLS - len) / 2;

        for (int col = 0; col < len; col++) {
            char ch = line[col];
            if (ch == ' ') continue;

            int px = (col_offset + col) * RET_FONT_WIDTH;
            int py = (start_row + row) * RET_FONT_HEIGHT;

            if (ch == '#' || ch == '@') {
                // Solid block (inverted space)
                ret_rend_draw_char(px, py, ' ', 1, RETGetFgColor());
            }
            else {
                // Use the character itself
                ret_rend_draw_char(px, py, ch, 0, RETGetFgColor());
            }
        }
    }

    // Draw name centered below the portrait
    int name_len = (int)strlen(name);
    int name_col = (PORTRAIT_COLS - name_len) / 2;
    int name_row = start_row + art_rows + 1;

    for (int i = 0; i < name_len; i++) {
        int px = (name_col + i) * RET_FONT_WIDTH;
        int py = name_row * RET_FONT_HEIGHT;
        ret_rend_draw_char(px, py, name[i], 0, RETGetFgColor());
    }

    RETSetFgColor(old_fg);
    RETSetFgBrightness(old_bright);

    RETRenderFrame();
}

void portrait_show_jobs(void) {
    draw_portrait(jobs_art, JOBS_ART_ROWS, "STEVE JOBS  1955-2011");
}

void portrait_show_wozniak(void) {
    draw_portrait(woz_art, WOZ_ART_ROWS, "STEVE WOZNIAK");
}
