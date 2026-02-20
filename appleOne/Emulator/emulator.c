#include <stdlib.h>

#include "m6502.h"
#include "pia6820.h"
#include "memory.h"
#include "keyboard.h"
#include "screen.h"
#include "statusbar.h"
#include "splash.h"
#include "../Effects/effect_ascii_art.h"

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
            effect_ascii_art_show_portrait(0, "steve-jobs");
            break;
        case 8:
            effect_ascii_art_show_portrait(0, "steve-wozniak");
            break;
        case 9:
            splash_skip();
            break;
        case 10:
            effect_ascii_art_show_portrait_pair("steve-jobs", "steve-wozniak");
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
