#ifndef RB_POSTPROCESS_H
#define RB_POSTPROCESS_H

#include "rb_display.h"

// Apply phosphor glow + scanline post-process.
// source: raw pixel buffer (RGBA, RB_PIXEL_WIDTH x RB_PIXEL_HEIGHT)
// dest:   output buffer with glow applied
void rb_postprocess_apply(byte *source, byte *dest);

// Get pointer to the internal glow output buffer
byte *rb_postprocess_get_buffer(void);

#endif
