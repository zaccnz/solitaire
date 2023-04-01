#pragma once

#include <toml.h>

typedef struct Licence
{
    char *name;
    char *source;
    char *author;
    char *buffer;
    char **lines;
    int line_count;
} Licence;

int licence_load(Licence *licence, toml_table_t *table);
void licence_free(Licence *licence);

void licences_load();
void licences_free();
Licence *licences_get(int *count);