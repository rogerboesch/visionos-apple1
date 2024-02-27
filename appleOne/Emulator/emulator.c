#include <stdlib.h>

#include "m6502.h"
#include "pia6820.h"
#include "memory.h"
#include "keyboard.h"
#include "screen.h"
#include "statusbar.h"

// Internal emulation functions (call trough emulator_task(number)

static void _load_basic_rom(void) {
    loadBasic();
    resetPia6820();
    resetM6502();
}

static void _load_startreck_core(void) {
    loadCore();
    resetPia6820();
    resetM6502();
}

static void _dump_core(void) {
    dumpCore();
    resetPia6820();
    resetM6502();
}

static void _soft_reset(void) {
    resetPia6820();
    resetM6502();
}

static void _hard_reset(void) {
    resetScreen();
    resetPia6820();
    resetMemory();
    resetM6502();
}

static void _flip_mode(void) {
    flipMode();
    resetPia6820();
    resetM6502();
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
        default:
            break;
    }
}

// Emulator calls

int emulator_init(void) {
	init_screen();
	statusbar_init();

	resetScreen();
    
    // 1M Hz. Sync emulation every 50 msec
	setSpeed(1000000, 50);

	if (!loadMonitor()) {
		statusbar_print("Failed to load monitor.rom");
		return 0;
	}

    resetMemory();

	resetM6502();
	startM6502();
    
    return 0;
}

int emulator_frame(void) {
    process_keyboard_input();
    return 0;
}

int emulator_stop(void) {
    stopM6502();
    return 0;
}
