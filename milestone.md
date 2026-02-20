# Milestone - Apple 50th Anniversary Feature Branch

## Key Summary
Building an immersive visionOS experience featuring the Apple I emulator with multiple display walls, ASCII art portraits of Steve Jobs and Steve Wozniak, and a standalone Breakout game sharing the same display pipeline.

## Completed Steps

### Breakout Game (2026-02-20)
- **New module `Games/game_breakout`**: Standalone Breakout game in C sharing display 0 (336x208, phosphor postprocess)
- **New files**: `game_breakout.h` (API + constants), `game_breakout.c` (state/physics), `game_breakout_render.c` (all drawing)
- **Playfield**: 10x7 brick grid (30x8 each, 2px gaps), 40x4 paddle, 4x4 ball, 4px walls, 24px HUD
- **Physics**: 3-substep loop per frame, AABB collision for walls/paddle/bricks, paddle spin based on hit offset
- **Rendering**: `fill_rect()` writes directly to `pixel_data` via `rb_get_display(0)`, HUD uses `rb_display_render_draw_char()`, different brightness per brick row for phosphor depth
- **Title screen**: Shows "BREAKOUT" + "PRESS LAUNCH TO START" with decorative bricks
- **Game over screen**: Shows final score + "PRESS RESET TO RETRY"
- **Level progression**: All bricks cleared → next level, bricks rebuild, ball auto-launches
- **ObjCBridge.m**: `app_mode` variable (0=emulator, 1=breakout) routes `EmulatorFrame()` to either `emulator_frame()` or `game_breakout_frame()`; `effect_ascii_art_frame()` always runs
- **Bridge functions**: `GameSetModeEmulator()`, `GameSetModeBreakout()`, `GameBreakoutInput()`, `GameBreakoutInputRelease()`, `GameBreakoutReset()`
- **MainWindow.swift**: `AppMode` enum, mode selector buttons (left of screen, radio-style green/white), conditional subtitle, context-dependent bottom buttons (emulator: RESET/BREAK/LOAD; breakout: LEFT/RIGHT/LAUNCH/RESET)
- **No changes** to display pipeline, ScreenRenderer, DisplayManager, GameSpace, terminal, or emulator — existing behavior fully preserved
- Builds successfully for visionOS simulator

### Refactoring: rb_display Unified Display Abstraction (2026-02-19)
- Combined `ret_renderer.c`/`.h` and `ret_textbuffer.c`/`.h` into a single `rb_display` system
- New `rb_display` struct contains: pixel buffer, text buffer, cursor, color state, graphics cursor, rendering flags — all per-display
- **New files**: `rb_display.h` (struct + API), `rb_display.c` (lifecycle), `rb_display_render.c` (pixel rendering), `rb_display_text.c` (text operations)
- **Deleted files**: `ret_renderer.h`, `ret_renderer.c`, `ret_textbuffer.h`, `ret_textbuffer.c`, `ret_platform_types.h`, `ret_palette.h`, `ret_palette.c`, `ret_font.h`, `ret_font_apple2.c`, `ret_font_amstrad.c`, `ret_font_pet.c`, `portraits.c`, `portraits.h`
- Up to 16 displays (`RB_DISPLAY_MAX`), each with malloc'd buffers, identified by index
- Display 0 is the main terminal (42x26, 336x208), created by `rb_display_init(42, 26)`
- Portrait rendering uses temporary displays via `rb_display_create()`/`rb_display_destroy()`
- All callers updated: `terminal.c`, `splash.c`, `portrait_hires.c`, `ObjCBridge.m`, `rb_postprocess.c`
- All `ret_*` naming eliminated — types, functions, constants, filenames all use `rb_` prefix
- `rb_display_init(cols, rows)` takes explicit dimensions (no more default constants)
- **All public functions take explicit `int display` parameter** — no active/current display concept
- Removed `rb_display_set_current()`/`rb_display_get_current()`/`rb_get_current()` and `rb_current_display` tracking
- `terminal.c` uses `TERMINAL_DISPLAY` (0), `splash.c` uses `SPLASH_DISPLAY` (0), `portrait_hires.c` passes temporary display index directly
- Palette, font, and platform types consolidated into `rb_display.h` / `rb_display_render.c`
- Postprocess renamed to `rb_postprocess.h`/`.c` with dynamic buffer sizing
- Builds successfully for visionOS simulator

### Font Consolidation + Text Layer for Splash/Portraits (2026-02-19)
- Embedded Apple II font array (`rb_font_apple2[]`) directly into `rb_display_render.c` as static const
- Deleted `ret_font.h`, `ret_font_apple2.c`, `ret_font_amstrad.c`, `ret_font_pet.c`
- Font defines (`RET_FONT_WIDTH`, `RET_FONT_HEIGHT`, etc.) moved into `rb_display.h`
- `splash.c` and `portrait_hires.c` now use `rb_display_text_print_char()` instead of `rb_display_render_draw_char()`
- Text and pixel buffers stay in sync — rendering goes through the text layer with `immediate_print` mode
- Splash logo uses `rb_display_set_invert(true/false)` for inverted space blocks

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

### Alternating Portraits + Carousel (2026-02-19)
- BOTH button renders Jobs and Wozniak, alternating across circle displays (even=Jobs, odd=Woz)
- Portrait pair pipeline: C renders both buffers → ObjCBridge creates two UIImages → `Renderer.renderPortraitPair` → `DisplayManager.updatePortraitPair` applies alternating materials
- Carousel rotation: displays orbit around center at `CAROUSEL_SPEED = 0.15 rad/s`, enabled by default
- ROTATE toggle button (green when active)

### Circle Display Modes — Radio Buttons (2026-02-19)
- `CircleDisplayMode` enum: `.mirror`, `.jobs`, `.woz`, `.both`
- MIRROR (default): circle displays show emulator output
- JOBS/WOZ: single portrait on all circle displays via `ret_render_portrait` path
- BOTH: alternating Jobs/Woz via `ret_render_portrait_pair` path
- Main panel always shows emulator — portraits never overwrite `screenImage`
- Radio button UI: active mode = green, others = white
- Round button style for side panel (56pt circle, 9pt font)
- Title changed to "TERMINAL", subtitle "Apple I Emulator" between display and controls

### Matrix Rain Effect for Portraits (2026-02-19)
- Three-phase animation triggered when JOBS or WOZ portrait is shown: hold (1s), rain down (1.5s), rebuild (1.5s)
- **New files**: `portrait_matrix.c` (implementation), `portrait_matrix.h` (API)
- Per-column animation with randomized speed (1-3 rows/frame) and start delay (0-15 frames) for organic look
- Rain phase: art chars shift down per-column, green matrix trail chars at leading edge with fading brightness
- Rebuild phase: art chars fall from above screen back to final position, snap when landed
- Uses existing persistent `portrait_display_a` slot via `portrait_hires_get_display_a()` getter
- State machine driven by `portrait_matrix_frame()` called from `emulator_frame()` at 60 FPS
- Pair mode: `portrait_matrix_start_pair()` runs two independent slot animations for BOTH mode
- Pair mode uses `rb_render_portrait_pair` bridge so alternating circle displays update together
- Per-slot RNG seeds differ so Jobs and Woz columns animate with different patterns
- `portrait_hires_get_display_b()` getter added for pair mode display access
- Auto-looping: after rebuild, 5-second pause on final portrait, then restarts with new seed — loops indefinitely
- `portrait_matrix_stop()` kills animation on MIRROR switch or hard reset
- Fixed buffer aliasing: `fromBuffer:` now copies pixel data via `CFDataCreate` so async Swift texture tasks read stable snapshots
- Tapping portrait button during animation restarts cleanly (re-initializes all state)
- Same phosphor-green tinting as normal portrait rendering
- Builds successfully for visionOS simulator

### Refactoring: Move portrait_ files to Effects/ folder (2026-02-20)
- Moved 6 portrait rendering files from `Emulator/` to `Emulator/Effects/` subfolder
- Renamed all files from `portrait_*` to `effect_*` prefix
- Renamed all public API functions: `portrait_hires_*` → `effect_hires_*`, `portrait_matrix_*` → `effect_matrix_*`
- Renamed data constants and header guards accordingly
- Updated all callers: `emulator.c`, `ObjCBridge.m`
- Added `Effects` group to Xcode project structure
- Git history preserved via `git mv`

### Refactoring: Generic Slot-Based Art Loader (2026-02-20)
- Replaced hardcoded Jobs/Wozniak portrait system with generic N-slot art loader
- **New module `effect_art_loader`**: Loads ASCII art from bundled `.txt` files at runtime (two-pass read: count lines + max width, then malloc + pad to uniform columns)
- **New module `effect_ascii_art`**: Slot-based display system with `EFFECT_ART_MAX_SLOTS` (8) slots. `effect_ascii_art_show(slot, name)` loads any art file into any slot
- **Refactored `effect_matrix`**: N independent animation slots instead of 2 hardcoded + `matrix_pair_mode` flag. `effect_matrix_start(slot, ...)` starts animation on a specific slot
- **Effect system fully independent of emulator**: `effect_ascii_art_frame()` ticked from `ObjCBridge.m` alongside `emulator_frame()` — matrix animation on circle displays never blocks the CPU emulator
- **Bridge calls owned by `effect_ascii_art`**: Matrix module only advances animation state; `effect_ascii_art` handles bridge push via `push_to_bridge()` internally
- **`emulator.c` clean**: Only calls `effect_ascii_art_show_portrait()`/`show_portrait_pair()` for task triggers and `effect_ascii_art_stop()` in hard reset — no bridge, matrix, or display refs
- **Deleted**: `effect_data_jobs.h`, `effect_data_wozniak.h` (compiled-in art headers), `effect_hires.h`, `effect_hires.c` (old hardcoded portrait renderer)
- **Added to bundle**: `steve-jobs.txt`, `steve-wozniak.txt` in `Assets/` folder as Resources
- Art getter API: `effect_ascii_art_get_lines/rows/cols(slot)` allow matrix to access loaded art data
- C layer is fully generic for N images; Swift bridge still supports 1 or 2 (single/pair)

### Previous Work
- Hi-res portrait renderer with phosphor-green display
- ASCII art portraits of Steve Jobs and Steve Wozniak
- Portrait data headers with density-shaded ASCII art
- Portrait buttons in the UI
