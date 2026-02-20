#ifndef EFFECT_ART_LOADER_H
#define EFFECT_ART_LOADER_H

#define ART_MAX_COLS 120
#define ART_MAX_ROWS 80

typedef struct {
    char **lines;
    int rows;
    int cols;
} effect_art;

int  effect_art_load(const char *name, effect_art *art);
void effect_art_free(effect_art *art);

#endif
