#include <stdlib.h>

#include "m6502.h"
#include "pia6820.h"
#include "memory.h"
#include "keyboard.h"
#include "screen.h"
#include "statusbar.h"
#include "splash.h"
#include "../Effects/effect_ascii_art.h"
#include "../Effects/effect_matrix.h"

/* Bridge functions declared in ObjCBridge.m */
void rb_render_portrait(unsigned char *data, int width, int height);
void rb_render_portrait_pair(unsigned char *dataA, int widthA, int heightA,
                              unsigned char *dataB, int widthB, int heightB);

/* Forward declaration for pixel data access */
unsigned char *rb_display_get_pixel_data(int display);
int rb_display_get_pixel_width(int display);
int rb_display_get_pixel_height(int display);

static int emulator_splash_done = 0;
static int emulator_cpu_started = 0;

// Internal emulation functions (call trough emulator_task(number)

static void _load_basic_rom(void) {
    memory_load_basic();
    pia6820_reset();
    m6502_reset();
}

static void _load_startreck_core(void) {
    memory_load_example_core();
    pia6820_reset();
    m6502_reset();
}

static void _dump_core(void) {
    memory_dump_core();
    pia6820_reset();
    m6502_reset();
}

static void _soft_reset(void) {
    pia6820_reset();
    m6502_reset();
}

static void _hard_reset(void) {
    effect_matrix_stop();
    effect_ascii_art_stop();
    m6502_stop();
    screen_reset();
    pia6820_reset();
    memory_reset();
    m6502_reset();

    splash_init();
    emulator_splash_done = 0;
    emulator_cpu_started = 0;
}

static void _flip_mode(void) {
    memory_flip_mode();
    pia6820_reset();
    m6502_reset();
}

static void _show_portrait(int slot, const char *name) {
    effect_ascii_art_show(slot, name);
    effect_matrix_start(slot,
                         effect_ascii_art_get_lines(slot),
                         effect_ascii_art_get_rows(slot),
                         effect_ascii_art_get_cols(slot));

    int d = effect_ascii_art_get_display(slot);
    if (d >= 0) {
        rb_render_portrait(rb_display_get_pixel_data(d),
                           rb_display_get_pixel_width(d),
                           rb_display_get_pixel_height(d));
    }
}

static void _show_portrait_pair(const char *name_a, const char *name_b) {
    effect_ascii_art_show(0, name_a);
    effect_ascii_art_show(1, name_b);
    effect_matrix_start(0,
                         effect_ascii_art_get_lines(0),
                         effect_ascii_art_get_rows(0),
                         effect_ascii_art_get_cols(0));
    effect_matrix_start(1,
                         effect_ascii_art_get_lines(1),
                         effect_ascii_art_get_rows(1),
                         effect_ascii_art_get_cols(1));

    int da = effect_ascii_art_get_display(0);
    int db = effect_ascii_art_get_display(1);
    if (da >= 0 && db >= 0) {
        rb_render_portrait_pair(rb_display_get_pixel_data(da),
                                rb_display_get_pixel_width(da),
                                rb_display_get_pixel_height(da),
                                rb_display_get_pixel_data(db),
                                rb_display_get_pixel_width(db),
                                rb_display_get_pixel_height(db));
    }
}

static void _push_portrait_to_bridge(void) {
    int count = effect_matrix_get_active_count();
    int d0 = effect_ascii_art_get_display(0);

    if (count == 1 && d0 >= 0) {
        rb_render_portrait(rb_display_get_pixel_data(d0),
                           rb_display_get_pixel_width(d0),
                           rb_display_get_pixel_height(d0));
    }
    else if (count >= 2) {
        int d1 = effect_ascii_art_get_display(1);
        if (d0 >= 0 && d1 >= 0) {
            rb_render_portrait_pair(rb_display_get_pixel_data(d0),
                                    rb_display_get_pixel_width(d0),
                                    rb_display_get_pixel_height(d0),
                                    rb_display_get_pixel_data(d1),
                                    rb_display_get_pixel_width(d1),
                                    rb_display_get_pixel_height(d1));
        }
    }
}

void emulator_task(int task) {
    switch (task) {
        case 1:
            _load_basic_rom();
            break;
        case 2:
            _load_startreck_core();
            break;
        case 3:
            _dump_core();
            break;
        case 4:
            _soft_reset();
            break;
        case 5:
            _hard_reset();
            break;
        case 6:
            _flip_mode();
            break;
        case 7:
            _show_portrait(0, "steve-jobs");
            break;
        case 8:
            _show_portrait(0, "steve-wozniak");
            break;
        case 9:
            splash_skip();
            break;
        case 10:
            _show_portrait_pair("steve-jobs", "steve-wozniak");
            break;
        default:
            break;
    }
}

// Emulator calls

int emulator_init(void) {
	screen_init();
	statusbar_init();

	screen_reset();

    // 1M Hz. Sync emulation every 50 msec
	m6502_set_speed(1000000, 50);

	if (!memory_load_wozmon()) {
		statusbar_print("Failed to load monitor.rom");
		return 0;
	}

    memory_reset();
	m6502_reset();

    // Start with splash instead of immediately booting the CPU
    splash_init();
    emulator_splash_done = 0;
    emulator_cpu_started = 0;

    return 0;
}

int emulator_frame(void) {
    /* Matrix rain animation takes priority when active */
    if (effect_matrix_is_active()) {
        effect_matrix_frame();
        _push_portrait_to_bridge();
        return 0;
    }

    // Run splash animation first
    if (!emulator_splash_done) {
        if (!splash_frame()) {
            emulator_splash_done = 1;
        }
        return 0;
    }

    // Start CPU once after splash finishes
    if (!emulator_cpu_started) {
        screen_reset();
        m6502_start();
        emulator_cpu_started = 1;
    }

    keyboard_process_input();
    return 0;
}

int emulator_stop(void) {
    m6502_stop();
    return 0;
}
