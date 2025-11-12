#include "text_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <sys/stat.h>

char *ReadFile(char const *const filename) {
    assert(filename);

    size_t text_size = 0;

    FILE *file = fopen(filename, "r");
    if (file == NULL)
        return NULL;

    struct stat file_stat = {};
    if (stat(filename, &file_stat) == -1)
        return NULL;

    size_t file_size = (size_t)file_stat.st_size;

    char *text = (char *)calloc(file_size + 1, sizeof(*text));
    if (text == NULL)
        return NULL;

    text_size = fread(text, sizeof(*text), file_size, file);
    text[text_size] = '\0';

    fclose(file);
    return text;
}
