#ifndef EFFECT_ASCII_ART_H
#define EFFECT_ASCII_ART_H

#define EFFECT_ART_MAX_SLOTS 8

int  effect_ascii_art_show(int slot, const char *name);
int  effect_ascii_art_get_display(int slot);
void effect_ascii_art_stop(void);

/* Getters for loaded art data (used by effect_matrix) */
const char **effect_ascii_art_get_lines(int slot);
int  effect_ascii_art_get_rows(int slot);
int  effect_ascii_art_get_cols(int slot);

#endif
