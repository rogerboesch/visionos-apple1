#ifndef EFFECT_MATRIX_H
#define EFFECT_MATRIX_H

void effect_matrix_start(int slot, const char **art, int rows, int cols);
void effect_matrix_stop(void);
void effect_matrix_stop_slot(int slot);
int  effect_matrix_frame(void);
int  effect_matrix_is_active(void);
int  effect_matrix_get_active_count(void);

#endif
