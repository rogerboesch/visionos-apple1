#ifndef RB_POSTPROCESS_H
#define RB_POSTPROCESS_H

#include "rb_display.h"

// Apply phosphor glow + scanline post-process.
// source: raw pixel buffer (RGBA, w x h)
// dest:   output buffer with glow applied
void rb_postprocess_apply(byte *source, byte *dest, int w, int h);

// Get pointer to the internal glow output buffer (allocates on first call)
byte *rb_postprocess_get_buffer(int w, int h);

#endif
