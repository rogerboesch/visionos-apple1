
#include "portraits.h"
#include "rb_display.h"

#include <string.h>

/* Screen grid: 42 columns x 26 rows */
#define PORTRAIT_COLS 42
#define PORTRAIT_ROWS 26

/* Steve Jobs - thin face, round glasses, short hair, turtleneck */
/* 24 rows x 34 cols, density-shaded ASCII */
#define JOBS_ART_ROWS 24
static const char *jobs_art[JOBS_ART_ROWS] = {
    "        .:-==++===-:.        ",
    "      :=*##%%%%%%%%#*=:      ",
    "    .=*%%%%%%%%%%%%%###*=.   ",
    "   :+#%%%%%##****##%%%%%+:   ",
    "  .+#%%%%*+==----==+*%%%#+.  ",
    "  :*%%%#+=:.      .:=+#%%*:  ",
    "  =#%%#+:   .=**=.   :+#%#= ",
    "  +#%%+.   =##%%##=   .+%%+ ",
    "  *%%%=   .*%#++#%*.   =#%* ",
    "  *%%%=   .+#*==*#+.   =#%* ",
    "  +#%%+.   :=++++=:   .+%%+ ",
    "  =#%%#+.     ..     .+#%#= ",
    "  :*%%%#=:   .==.   :=#%%*: ",
    "  .+#%%%%+=-+####+-=+%%%#+. ",
    "   :+#%%%%%########%%%%%+:  ",
    "    .=*%%%%%%%##%%%%%%%*=.  ",
    "      :=*##%%%%%%%%%%*=:    ",
    "     .:-=+*###%%###*+=-:.   ",
    "    :+#%%%%%%%%%%%%%%%%%#+: ",
    "   .+#%%%%%%%%%%%%%%%%%%%+. ",
    "   =#%%%%%%%%%%%%%%%%%%%%%# ",
    "   *%%%%%%%%%%%%%%%%%%%%%#* ",
    "   *%%%%%%%%%%%%%%%%%%%%%#* ",
    "   =#%%%%%%%%%%%%%%%%%%%#%= "
};

/* Steve Wozniak - rounder face, beard, curly hair, glasses */
/* 24 rows x 34 cols, density-shaded ASCII */
#define WOZ_ART_ROWS 24
static const char *woz_art[WOZ_ART_ROWS] = {
    "      .-=+**####**+=-..     ",
    "    :=*####%%%%%%%%%##*=:   ",
    "   =*##%%%%%%%%%%%%%%%%%*=  ",
    "  :*#%%%%%%%%%%%%%%%%%%%%*: ",
    "  +#%%%%%####**####%%%%%#+  ",
    "  *%%%%#+=:.    .:=+#%%%%*  ",
    " :#%%%+: .=*##*=. :+%%%#:  ",
    " :#%%+. :#%%%%%%#: .+%%#:  ",
    " :#%%=  +%%#++#%%+  =%%#:  ",
    " :#%%=  =##*==*##=  =%%#:  ",
    " :#%%+. :=*#%%#*=: .+%%#:  ",
    " :#%%%+:  .:==:.  :+%%%#:  ",
    "  *%%%%=:  :==:  :=%%%%*   ",
    "  +#%%%%+==*%%*==+%%%%#+   ",
    "  :*#%%%%%######%%%%%#*:   ",
    "   =*%%*+*######*+*%%*=    ",
    "   :+#%*=+#%%%%#+=#%#+:    ",
    "   .+#%%####%%####%%#+.    ",
    "    =#%%%%%%%%%%%%#%%%=    ",
    "    :*#%%%%%##%%%%%#*:     ",
    "     :=+*########*+=:      ",
    "    .=*##%%%%##%%%%##*=.   ",
    "   :+#%%%%%%%%%%%%%%%%%%+: ",
    "   =#%%%%%%%%%%%%%%%%%%%%#="
};

/* Draw a portrait centered on screen with name below */
static void draw_portrait(const char **art, int art_rows,
                          const char *name) {
    rb_display_render_clear();

    byte old_fg = rb_display_set_fg_color(RET_COLOR_GREEN);
    byte old_bright = rb_display_set_fg_brightness(15);

    /* Center the art vertically (leave room for name at bottom) */
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

            rb_display_render_draw_char(px, py, ch, 0, rb_display_get_fg_color());
        }
    }

    /* Draw name centered below the portrait */
    int name_len = (int)strlen(name);
    int name_col = (PORTRAIT_COLS - name_len) / 2;
    int name_row = start_row + art_rows;

    for (int i = 0; i < name_len; i++) {
        int px = (name_col + i) * RET_FONT_WIDTH;
        int py = name_row * RET_FONT_HEIGHT;
        rb_display_render_draw_char(px, py, name[i], 0, rb_display_get_fg_color());
    }

    rb_display_set_fg_color(old_fg);
    rb_display_set_fg_brightness(old_bright);

    rb_display_render_frame();
}

void portrait_show_jobs(void) {
    draw_portrait(jobs_art, JOBS_ART_ROWS,
                  "STEVE JOBS  1955-2011");
}

void portrait_show_wozniak(void) {
    draw_portrait(woz_art, WOZ_ART_ROWS,
                  "STEVE WOZNIAK");
}
