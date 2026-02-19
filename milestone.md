# Milestone - Apple 50th Anniversary Feature Branch

## Key Summary
Building an immersive visionOS experience featuring the Apple I emulator with multiple display walls and ASCII art portraits of Steve Jobs and Steve Wozniak.

## Completed Steps

### Circle Display Layout (2026-02-19)
- Added `placeDisplayCircle()` to `DisplayManager.swift` that places 8 displays in a 10m radius circle around the user at startup
- Constants: `CIRCLE_DISPLAY_COUNT = 8`, `CIRCLE_RADIUS = 10.0m`
- Bumped `DISPLAY_MAX_COUNT` from 10 to 18 (8 circle + room for manual placements)
- Each display faces inward using `simd_quatf(angle:axis:)` rotation
- Circle centers on user head position (falls back to origin if tracking unavailable)
- Called once from `GameSpace.swift` after `startTracking()`, before the head-follow loop
- Manual "PLACE DISPLAY" button still works for additional displays
- All displays receive texture updates via existing `updateTexture()` iteration

### Dynamic Text Buffer + Character-Based Hires Portraits (2026-02-19)
- Made `ret_textbuffer.c` dynamic: runtime `ret_text_width`/`ret_text_height` with `ret_text_resize(cols, rows)` and `ret_text_restore_default()`
- Static default buffers (42x26) remain for normal operation; pointers switch to malloc'd memory when resized
- Added `ret_rend_set_custom_target(buffer, w, h)` / `ret_rend_restore_target()` to renderer for custom pixel buffer rendering
- `ret_rend_draw_char()` now uses runtime stride instead of compile-time `RET_PIXEL_WIDTH`
- Rewrote `portrait_hires.c`: renders portraits with actual Apple II bitmap font glyphs via `ret_rend_draw_char()` instead of 3x3 colored pixel blocks
- Jobs buffer: 960x512 (120 cols * 8px, 64 rows * 8px), Woz buffer: 960x616 (120 cols * 8px, 77 rows * 8px)
- Removed `PORTRAIT_PIXEL_SCALE`, `char_to_brightness()`, phosphor color constants
- `emulator.c` task 7/8 now calls `portrait_hires_show_*()` directly

### Phosphor Color Fix for Portraits (2026-02-19)
- Portraits were rendering with palette `RET_COLOR_GREEN` (0,204,85) but normal text goes through `ret_postprocess.c` which tints to phosphor green (51,255,51)
- Fixed by rendering portrait glyphs as white, then tinting the entire buffer to phosphor green with `tint_buffer_phosphor()`

### Immersive Space Control Panel (2026-02-19)
- Moved control panel from flat SwiftUI window into RealityKit immersive space as an attachment
- `MainWindow` is now a thin launcher (inits emulator, opens immersive space, dismisses itself)
- `ControlPanel` lives as `Attachment(id: "control_panel")` in `GameSpace.swift`
- Panel tilted 30 degrees like a table using quaternion composition (yaw + tilt)
- Billboard rotation: yaw-only facing via `atan2`, smooth slerp interpolation
- Panel placed once at startup (`placePanel()`), does NOT follow user — only rotates (`updatePanelBillboard()`)
- Shockwave-inspired colored buttons: RESET=red, BREAK=orange, LOAD=cyan, START=green, TYPE/RUN=blue, Portraits=purple

### Splash Screen Hold Until BREAK (2026-02-19)
- Splash fades in over 1.5s then holds at full brightness indefinitely (removed auto-advance timer)
- BREAK button calls `EmulatorSkipSplash()` → `emulator_task(9)` → `splash_skip()` which triggers fade-out
- Fade-out takes 0.75s, then emulator boots the CPU normally
- Added `splash_fading_out` / `splash_fadeout_frame` state tracking

### Previous Work
- Hi-res portrait renderer with phosphor-green display
- ASCII art portraits of Steve Jobs and Steve Wozniak
- Portrait data headers with density-shaded ASCII art
- Portrait buttons in the UI
