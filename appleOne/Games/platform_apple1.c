/*
 * platform_apple1.c
 *
 * Shockwave platform implementation for the Apple I display pipeline.
 * Renders into display 0's pixel buffer (336x208) which gets the
 * phosphor green postprocess applied automatically.
 */

#include "platform.h"
#include "input.h"
#include "system.h"
#include "utils.h"
#include "mem.h"
#include "packfile.h"

#include "rb_display.h"

#include <mach/mach_time.h>
#include <string.h>

/* Display 0 access from rb_display.c */
extern rb_display *rb_get_display(int index);

/* Forward declaration for rb_display_render_frame (rb_display_render.c) */
extern void rb_display_render_frame(int display);

/* -------------------------------------------------------------------------- */
/*  State                                                                     */
/* -------------------------------------------------------------------------- */

static bool wants_to_exit = false;
static void (*audio_callback)(float *buffer, uint32_t len) = NULL;

static char path_assets[512];
static char path_userdata[512];
static char path_exe_dir[512];

long g_line_count = 0;

static mach_timebase_info_data_t timebase_info;
static bool timebase_initialized = false;

/* Apple I display 0 dimensions */
#define APPLE1_SCREEN_W 336
#define APPLE1_SCREEN_H 208

/* -------------------------------------------------------------------------- */
/*  Timing                                                                    */
/* -------------------------------------------------------------------------- */

double platform_now(void) {
	if (!timebase_initialized) {
		mach_timebase_info(&timebase_info);
		timebase_initialized = true;
	}
	uint64_t now = mach_absolute_time();
	double nanos = (double)now * (double)timebase_info.numer / (double)timebase_info.denom;
	return nanos / 1.0e9;
}

/* -------------------------------------------------------------------------- */
/*  Screen                                                                    */
/* -------------------------------------------------------------------------- */

rgba_t *platform_get_screenbuffer(int32_t *pitch) {
	rb_display *d = rb_get_display(0);
	*pitch = d->pixel_width * (int32_t)sizeof(rgba_t);
	return (rgba_t *)d->pixel_data;
}

vec2i_t platform_screen_size(void) {
	return vec2i(APPLE1_SCREEN_W, APPLE1_SCREEN_H);
}

bool platform_get_fullscreen(void) {
	return true;
}

void platform_set_fullscreen(bool fullscreen) {
	(void)fullscreen;
}

/* -------------------------------------------------------------------------- */
/*  Line drawing (Bresenham into display 0 pixel buffer)                      */
/* -------------------------------------------------------------------------- */

void platform_line(vec2i_t p0, vec2i_t p1, rgba_t color) {
	rb_display *d = rb_get_display(0);
	int w = d->pixel_width;
	int h = d->pixel_height;
	byte *pixels = d->pixel_data;

	/* Trivial reject */
	if ((p0.x < 0 && p1.x < 0) || (p0.x >= w && p1.x >= w)) return;
	if ((p0.y < 0 && p1.y < 0) || (p0.y >= h && p1.y >= h)) return;

	/* Use grayscale from green channel (matches render_software.c) */
	byte gray = color.g;

	/* Bresenham */
	int32_t dx = abs(p1.x - p0.x);
	int32_t dy = abs(p1.y - p0.y);
	int32_t sx = p0.x < p1.x ? 1 : -1;
	int32_t sy = p0.y < p1.y ? 1 : -1;
	int32_t err = dx - dy;

	while (1) {
		if (p0.x >= 0 && p0.x < w && p0.y >= 0 && p0.y < h) {
			int off = (p0.y * w + p0.x) * 4;
			pixels[off]     = gray;
			pixels[off + 1] = gray;
			pixels[off + 2] = gray;
			pixels[off + 3] = 255;
		}

		if (p0.x == p1.x && p0.y == p1.y) break;

		int32_t e2 = 2 * err;
		if (e2 > -dy) { err -= dy; p0.x += sx; }
		if (e2 < dx)  { err += dx; p0.y += sy; }
	}

	g_line_count++;
}

/* -------------------------------------------------------------------------- */
/*  HUD                                                                       */
/* -------------------------------------------------------------------------- */

bool platform_hud_is_enabled(void) {
	return true;
}

/* -------------------------------------------------------------------------- */
/*  Exit                                                                      */
/* -------------------------------------------------------------------------- */

void platform_exit(void) {
	wants_to_exit = true;
}

/* -------------------------------------------------------------------------- */
/*  Paths                                                                     */
/* -------------------------------------------------------------------------- */

const char *platform_get_exe_dir(void) {
	return path_exe_dir;
}

/* -------------------------------------------------------------------------- */
/*  Audio                                                                     */
/* -------------------------------------------------------------------------- */

void platform_set_audio_mix_cb(void (*cb)(float *buffer, uint32_t len)) {
	audio_callback = cb;
}

/* -------------------------------------------------------------------------- */
/*  Asset loading                                                             */
/* -------------------------------------------------------------------------- */

FILE *platform_open_asset(const char *name, const char *mode) {
	char path[1024];
	snprintf(path, sizeof(path), "%s%s", path_assets, name);
	return fopen(path, mode);
}

uint8_t *platform_load_asset(const char *name, uint32_t *bytes_read) {
	return packfile_load(name, bytes_read);
}

/* -------------------------------------------------------------------------- */
/*  User data                                                                 */
/* -------------------------------------------------------------------------- */

uint8_t *platform_load_userdata(const char *name, uint32_t *bytes_read) {
	char path[1024];
	snprintf(path, sizeof(path), "%s%s", path_userdata, name);
	if (!file_exists(path)) {
		*bytes_read = 0;
		return NULL;
	}
	return file_load(path, bytes_read);
}

uint32_t platform_store_userdata(const char *name, void *bytes, int32_t len) {
	char path[1024];
	snprintf(path, sizeof(path), "%s%s", path_userdata, name);
	return file_store(path, bytes, len);
}

/* -------------------------------------------------------------------------- */
/*  Bridge entry points (called from ObjCBridge.m)                            */
/* -------------------------------------------------------------------------- */

static bool shockwave_initialized = false;

void shockwave_apple1_init(const char *asset_path, const char *userdata_path) {
	if (shockwave_initialized) return;

	snprintf(path_exe_dir, sizeof(path_exe_dir), "%s/", asset_path);
	snprintf(path_assets, sizeof(path_assets), "%s/", asset_path);
	snprintf(path_userdata, sizeof(path_userdata), "%s/", userdata_path);

	system_init();
	shockwave_initialized = true;
}

void shockwave_apple1_update(void) {
	if (!shockwave_initialized) return;

	g_line_count = 0;
	system_update();
	rb_display_render_frame(0);
}

void shockwave_apple1_input(int button, float state) {
	if (button > 0 && button < INPUT_BUTTON_MAX) {
		input_set_button_state((button_t)button, state);
	}
}

void shockwave_apple1_reset(void) {
	if (!shockwave_initialized) return;
	/* Reset to intro scene — game will cycle back to attract mode */
	extern void game_set_scene(int scene);
	game_set_scene(0);
}
