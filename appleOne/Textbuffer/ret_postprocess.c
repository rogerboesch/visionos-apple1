
#include "ret_postprocess.h"
#include "rb_display.h"

#include <string.h>
#include <math.h>

// Phosphor green color (classic P1 phosphor)
#define PHOSPHOR_R 51
#define PHOSPHOR_G 255
#define PHOSPHOR_B 51

// Glow radius in pixels
#define GLOW_RADIUS 3

// Glow intensity (0.0 - 1.0)
#define GLOW_INTENSITY 0.45f

// Scanline darkening (0.0 = no effect, 1.0 = full black lines)
#define SCANLINE_STRENGTH 0.15f

// Brightness threshold to count as "lit" pixel
#define LIT_THRESHOLD 30

// Output buffer with glow applied
static byte ret_glow_buffer[RB_PIXEL_WIDTH * RB_PIXEL_HEIGHT * 4];

// Pre-computed glow kernel
static float glow_kernel[GLOW_RADIUS * 2 + 1][GLOW_RADIUS * 2 + 1];
static int glow_kernel_ready = 0;

// -----------------------------------------------------------------------------
#pragma mark - Kernel setup

static void build_glow_kernel(void) {
    if (glow_kernel_ready) {
        return;
    }

    float sigma = GLOW_RADIUS / 2.0f;
    float sum = 0.0f;

    for (int dy = -GLOW_RADIUS; dy <= GLOW_RADIUS; dy++) {
        for (int dx = -GLOW_RADIUS; dx <= GLOW_RADIUS; dx++) {
            float dist = sqrtf((float)(dx * dx + dy * dy));
            float val = expf(-(dist * dist) / (2.0f * sigma * sigma));
            glow_kernel[dy + GLOW_RADIUS][dx + GLOW_RADIUS] = val;
            sum += val;
        }
    }

    // Normalize
    for (int dy = 0; dy < GLOW_RADIUS * 2 + 1; dy++) {
        for (int dx = 0; dx < GLOW_RADIUS * 2 + 1; dx++) {
            glow_kernel[dy][dx] /= sum;
        }
    }

    glow_kernel_ready = 1;
}

// -----------------------------------------------------------------------------
#pragma mark - Post-process

static inline int clamp_byte(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return v;
}

void ret_postprocess_apply(byte *source, byte *dest) {
    build_glow_kernel();

    int w = RB_PIXEL_WIDTH;
    int h = RB_PIXEL_HEIGHT;
    int stride = w * 4;

    // Start with black background
    memset(dest, 0, w * h * 4);

    // Pass 1: Apply glow from lit pixels
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int src_off = (y * w + x) * 4;
            int brightness = source[src_off];  // R channel as brightness proxy

            if (source[src_off + 1] > brightness) {
                brightness = source[src_off + 1];  // G
            }
            if (source[src_off + 2] > brightness) {
                brightness = source[src_off + 2];  // B
            }

            if (brightness < LIT_THRESHOLD) {
                continue;
            }

            float intensity = (float)brightness / 255.0f;

            // Spread glow to neighbors
            for (int dy = -GLOW_RADIUS; dy <= GLOW_RADIUS; dy++) {
                int ny = y + dy;
                if (ny < 0 || ny >= h) continue;

                for (int dx = -GLOW_RADIUS; dx <= GLOW_RADIUS; dx++) {
                    int nx = x + dx;
                    if (nx < 0 || nx >= w) continue;

                    float kernel_val = glow_kernel[dy + GLOW_RADIUS][dx + GLOW_RADIUS];
                    float glow = kernel_val * intensity * GLOW_INTENSITY;

                    int dst_off = (ny * w + nx) * 4;
                    int add_r = (int)(PHOSPHOR_R * glow);
                    int add_g = (int)(PHOSPHOR_G * glow);
                    int add_b = (int)(PHOSPHOR_B * glow);

                    dest[dst_off]     = clamp_byte(dest[dst_off] + add_r);
                    dest[dst_off + 1] = clamp_byte(dest[dst_off + 1] + add_g);
                    dest[dst_off + 2] = clamp_byte(dest[dst_off + 2] + add_b);
                    dest[dst_off + 3] = 255;
                }
            }
        }
    }

    // Pass 2: Add original pixels on top (tinted phosphor green) + scanlines
    for (int y = 0; y < h; y++) {
        float scanline = 1.0f;
        if (y % 2 == 1) {
            scanline = 1.0f - SCANLINE_STRENGTH;
        }

        for (int x = 0; x < w; x++) {
            int off = (y * w + x) * 4;
            int brightness = source[off];
            if (source[off + 1] > brightness) brightness = source[off + 1];
            if (source[off + 2] > brightness) brightness = source[off + 2];

            if (brightness >= LIT_THRESHOLD) {
                // Tint to phosphor green and add on top of glow
                float t = (float)brightness / 255.0f;
                int pr = (int)(PHOSPHOR_R * t);
                int pg = (int)(PHOSPHOR_G * t);
                int pb = (int)(PHOSPHOR_B * t);

                dest[off]     = clamp_byte(pr + dest[off] / 2);
                dest[off + 1] = clamp_byte(pg + dest[off + 1] / 2);
                dest[off + 2] = clamp_byte(pb + dest[off + 2] / 2);
            }

            // Apply scanline
            dest[off]     = (byte)(dest[off] * scanline);
            dest[off + 1] = (byte)(dest[off + 1] * scanline);
            dest[off + 2] = (byte)(dest[off + 2] * scanline);
            dest[off + 3] = 255;
        }
    }
}

byte *ret_postprocess_get_buffer(void) {
    return ret_glow_buffer;
}
