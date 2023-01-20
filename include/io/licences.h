#pragma once

typedef struct Licence
{
    char *name;
    char *source;
    char *author;
    char *buffer;
    char **lines;
    int line_count;
} Licence;

void licences_load();
void licences_free();
Licence *licences_get(int *count);