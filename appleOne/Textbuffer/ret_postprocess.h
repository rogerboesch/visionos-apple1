#ifndef RET_POSTPROCESS_H
#define RET_POSTPROCESS_H

#include "rb_display.h"

// Apply phosphor glow + scanline post-process.
// source: raw pixel buffer (RGBA, RET_PIXEL_WIDTH x RET_PIXEL_HEIGHT)
// dest:   output buffer with glow applied
void ret_postprocess_apply(byte *source, byte *dest);

// Get pointer to the internal glow output buffer
byte *ret_postprocess_get_buffer(void);

#endif
