#ifndef EFFECT_ASCII_ART_H
#define EFFECT_ASCII_ART_H

#define EFFECT_ART_MAX_SLOTS 8

/* Slot management */
int  effect_ascii_art_show(int slot, const char *name);
int  effect_ascii_art_get_display(int slot);
void effect_ascii_art_stop(void);

/* Getters for loaded art data (used by effect_matrix) */
const char **effect_ascii_art_get_lines(int slot);
int  effect_ascii_art_get_rows(int slot);
int  effect_ascii_art_get_cols(int slot);

/* High-level portrait commands */
void effect_ascii_art_show_portrait(int slot, const char *name);
void effect_ascii_art_show_portrait_pair(const char *name_a, const char *name_b);

/* Frame tick — advances matrix animation + pushes to bridge. Call from main loop. */
void effect_ascii_art_frame(void);

#endif
