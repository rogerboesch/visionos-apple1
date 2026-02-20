#ifndef EFFECT_MATRIX_H
#define EFFECT_MATRIX_H

void effect_matrix_start(const char **art, int rows, int cols);
void effect_matrix_start_pair(const char **art_a, int rows_a, int cols_a,
                                const char **art_b, int rows_b, int cols_b);
void effect_matrix_stop(void);
int effect_matrix_frame(void);
int effect_matrix_is_active(void);

#endif
