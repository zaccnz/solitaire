#include "io/licences.h"

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <toml.h>

Licence *licences;
int licence_count;

int licence_lines(Licence *licence)
{
    int licence_length = strlen(licence->buffer);

    int lines = 1;
    for (int i = 0; i < licence_length; i++)
    {
        if (licence->buffer[i] == '\n')
        {
            lines++;
        }
    }

    licence->lines = malloc(sizeof(char *) * lines);
    licence->line_count = lines;

    int line_ptr = 0;
    char *start = licence->buffer;
    for (int i = 0; i < licence_length; i++)
    {
        if (licence->buffer[i] == '\n')
        {
            licence->buffer[i] = 0;

            // CRLF check
            if (i > 0 && licence->buffer[i - 1] == '\r')
            {
                licence->buffer[i - 1] = 0;
            }

            licence->lines[line_ptr++] = start;
            start = &licence->buffer[i + 1];

            if (line_ptr >= lines)
            {
                printf("failed to parse lines from licence %s, too many\n", licence->name);
                free(licence->lines);
                return 0;
            }
        }
    }

    licence->lines[line_ptr++] = start;

    if (line_ptr != lines)
    {
        printf("failed to parse lines from licence %s, some missing\n", licence->name);
        free(licence->lines);
        return 0;
    }

    return 1;
}

int licence_load(Licence *licence, toml_table_t *table)
{
    toml_datum_t name = toml_string_in(table, "name");
    if (!name.ok)
    {
        printf("licence missing name!\n");
        return 0;
    }
    licence->name = name.u.s;

    toml_datum_t source = toml_string_in(table, "source");
    if (source.ok)
    {
        licence->source = source.u.s;
    }

    toml_datum_t author = toml_string_in(table, "author");
    if (author.ok)
    {
        licence->author = author.u.s;
    }

    toml_datum_t licence_string = toml_string_in(table, "licence");
    if (!licence_string.ok)
    {
        free(name.u.s);
        if (licence->source)
        {
            free(licence->source);
            licence->source = NULL;
        }

        if (licence->author)
        {
            free(licence->author);
            licence->author = NULL;
        }

        return 0;
    }

    licence->buffer = licence_string.u.s;

    return licence_lines(licence);
}

void licences_load()
{
    char *data = LoadFileText("res/licences.toml");

    if (!data)
    {
        printf("failed to read res/licences.toml\n");
        return;
    }

    char errbuf[200];
    toml_table_t *licences_toml = toml_parse(data, errbuf, sizeof(errbuf));
    UnloadFileText(data);

    if (!licences_toml)
    {
        printf("cannot parse res/licences.toml: %s\n", errbuf);
        return;
    }

    toml_array_t *licences_arr = toml_array_in(licences_toml, "licences");
    if (!licences_arr)
    {
        printf("licences.toml has no array called licences\n");
        return;
    }

    int length = toml_array_nelem(licences_arr);
    licences = malloc(sizeof(Licence) * length);
    memset(licences, 0, sizeof(Licence) * length);

    for (int i = 0; i < length; i++)
    {
        toml_table_t *licence = toml_table_at(licences_arr, i);

        if (!licence)
        {
            printf("[[licences]] at index %d not a table\n", i);
            continue;
        }

        if (!licence_load(&licences[licence_count], licence))
        {
            printf("[[licences]] at index %d failed to parse\n", i);
            continue;
        }

        licence_count++;
    }

    toml_free(licences_toml);
}

void licences_free()
{
    for (int i = 0; i < licence_count; i++)
    {
        Licence *licence = licences + i;
        free(licence->name);
        free(licence->buffer);
        free(licence->lines);
        if (licence->source)
        {
            free(licence->source);
        }
        if (licence->author)
        {
            free(licence->author);
        }
    }

    free(licences);
    licences = NULL;
    licence_count = 0;
}

Licence *licences_get(int *count)
{
    *count = licence_count;
    return licences;
}