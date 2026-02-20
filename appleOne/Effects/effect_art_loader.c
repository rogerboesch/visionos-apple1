#include "effect_art_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Platform function — returns static buffer with full path to bundled resource */
char *platform_file_path(char *name, char *extension);

int effect_art_load(const char *name, effect_art *art) {
    if (!art) return 0;

    char *path = platform_file_path((char *)name, "txt");
    if (!path) return 0;

    FILE *fd = fopen(path, "r");
    if (!fd) return 0;

    /* Pass 1: count lines and find max width */
    char buf[ART_MAX_COLS + 2];
    int row_count = 0;
    int max_width = 0;

    while (fgets(buf, sizeof(buf), fd) && row_count < ART_MAX_ROWS) {
        /* Strip trailing newline/carriage return */
        int len = (int)strlen(buf);
        while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
            len--;
        }
        if (len > ART_MAX_COLS) len = ART_MAX_COLS;
        if (len > max_width) max_width = len;
        row_count++;
    }

    if (row_count == 0) {
        fclose(fd);
        return 0;
    }

    /* Allocate line pointer array */
    art->lines = (char **)malloc(row_count * sizeof(char *));
    if (!art->lines) {
        fclose(fd);
        return 0;
    }
    art->rows = row_count;
    art->cols = max_width;

    /* Pass 2: rewind and read lines, pad to uniform width */
    rewind(fd);
    for (int r = 0; r < row_count; r++) {
        art->lines[r] = (char *)malloc(max_width + 1);
        if (!art->lines[r]) {
            /* Clean up on allocation failure */
            for (int j = 0; j < r; j++) free(art->lines[j]);
            free(art->lines);
            art->lines = NULL;
            art->rows = 0;
            art->cols = 0;
            fclose(fd);
            return 0;
        }

        if (fgets(buf, sizeof(buf), fd)) {
            int len = (int)strlen(buf);
            while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r')) {
                len--;
            }
            if (len > max_width) len = max_width;

            memcpy(art->lines[r], buf, len);
            /* Pad shorter lines with spaces */
            for (int c = len; c < max_width; c++) {
                art->lines[r][c] = ' ';
            }
        }
        else {
            /* Shouldn't happen, but pad with spaces */
            memset(art->lines[r], ' ', max_width);
        }
        art->lines[r][max_width] = '\0';
    }

    fclose(fd);
    return 1;
}

void effect_art_free(effect_art *art) {
    if (!art || !art->lines) return;

    for (int r = 0; r < art->rows; r++) {
        free(art->lines[r]);
    }
    free(art->lines);
    art->lines = NULL;
    art->rows = 0;
    art->cols = 0;
}
