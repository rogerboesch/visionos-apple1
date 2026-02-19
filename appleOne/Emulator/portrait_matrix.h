#ifndef PORTRAIT_MATRIX_H
#define PORTRAIT_MATRIX_H

void portrait_matrix_start(const char **art, int rows, int cols);
void portrait_matrix_start_pair(const char **art_a, int rows_a, int cols_a,
                                const char **art_b, int rows_b, int cols_b);
void portrait_matrix_stop(void);
int portrait_matrix_frame(void);
int portrait_matrix_is_active(void);

#endif
