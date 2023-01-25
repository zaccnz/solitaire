#include "util/toml_writer.h"

#include <raylib.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_INITIAL_SIZE 2048

/* UTILITY FUNCTIONS */
int toml_writer_check_resize(TOML_Writer *writer, int append_len)
{
    if (writer->buffer_len + append_len >= writer->buffer_size)
    {
        int new_size = writer->buffer_size * 2;
        writer->buffer = realloc(writer->buffer, new_size);
        if (!writer->buffer)
        {
            printf("toml_writer: failed to allocate %d bytes for buffer\n", new_size);
            return 0;
        }
        memset(writer->buffer + writer->buffer_size, 0, writer->buffer_size);
        writer->buffer_size = new_size;
    }
    return 1;
}

int toml_writer_putc(TOML_Writer *writer, char character)
{
    toml_writer_check_resize(writer, 1);
    writer->buffer[writer->buffer_len++] = character;
}

int toml_writer_indent(TOML_Writer *writer)
{
    int pad = (writer->current ? writer->current->parent_count : 0) * 2;
    for (int i = 0; i < pad; i++)
    {
        toml_writer_putc(writer, ' ');
    }
}

// note: please only write one line at a time, otherwise padding will be broken
int toml_writer_printf(TOML_Writer *writer, char *format, ...)
{
    va_list args;
    char buffer[BUFFER_INITIAL_SIZE / 2];
    int ret;

    va_start(args, format);
    ret = vsnprintf(buffer, BUFFER_INITIAL_SIZE / 2, format, args);
    va_end(args);

    if (ret < 0)
    {
        return 0;
    }

    int length = strlen(buffer);

    toml_writer_check_resize(writer, length);

    memcpy(writer->buffer + writer->buffer_len, buffer, length);
    writer->buffer_len += length;

    return 1;
}

void toml_writer_prepare(TOML_Writer *writer)
{
    if (writer->do_whitespace)
    {
        toml_writer_putc(writer, '\n');
        writer->do_whitespace = 0;
    }
    writer->key_do_whitespace = 1;
}

/* MAIN */

TOML_Writer *toml_writer_new()
{
    TOML_Writer *writer = malloc(sizeof(TOML_Writer));
    memset(writer, 0, sizeof(TOML_Writer));

    writer->buffer = malloc(BUFFER_INITIAL_SIZE);
    memset(writer->buffer, 0, BUFFER_INITIAL_SIZE);
    writer->buffer_size = BUFFER_INITIAL_SIZE;

    return writer;
}

void toml_writer_free(TOML_Writer *writer)
{
    TOML_Writer_Key *current = writer->current;
    while (current != NULL)
    {
        TOML_Writer_Key *next = current->parent;
        free(current->name);
        free(current);
        current = next;
    }

    free(writer->buffer);
    free(writer);
}

int toml_writer_save(TOML_Writer *writer, char *filename)
{
    return SaveFileText(filename, writer->buffer);
}

int toml_writer_push_key(TOML_Writer *writer, char *key, int array)
{
    TOML_Writer_Key *writer_key = malloc(sizeof(TOML_Writer_Key));
    memset(writer_key, 0, sizeof(TOML_Writer_Key));

    int key_len = strlen(key) + 1;
    writer_key->name = malloc(key_len);
    strncpy(writer_key->name, key, key_len);
    writer_key->is_array = array;

    if (writer->current != NULL)
    {
        TOML_Writer_Key *parent = writer->current;
        writer_key->parent = parent;
        writer_key->parent_count = parent->parent_count + 1;
    }
    else
    {
        writer_key->parent_count = 0;
    }

    writer->current = writer_key;
    if (writer->key_do_whitespace)
    {
        toml_writer_putc(writer, '\n');
    }
    writer->key_do_whitespace = 0;
    writer->do_whitespace = 0;

    char full_key[2048] = {0};
    int full_key_ptr = 2046;
    TOML_Writer_Key *current = writer_key;
    while (current != NULL)
    {
        full_key_ptr--;
        int len = strlen(current->name);
        full_key_ptr -= len;
        strncpy(full_key + full_key_ptr, current->name, len);
        full_key[full_key_ptr - 1] = '.';
        current = current->parent;
    }

    const char *FORMAT_STRING = array ? "[[%s]]\n" : "[%s]\n";
    toml_writer_indent(writer);
    toml_writer_printf(writer, FORMAT_STRING, &full_key[full_key_ptr]);

    return 1;
}

int toml_writer_pop_key(TOML_Writer *writer)
{
    if (writer->current == NULL)
    {
        printf("toml_writer: unable to pop key - no key pushed\n");
        return 0;
    }

    TOML_Writer_Key *current = writer->current;
    writer->current = current->parent;
    free(current);

    writer->do_whitespace = 1;

    return 1;
}

int toml_writer_push_value_array(TOML_Writer *writer, char *key)
{
    writer->is_writing_value_array = 1;
    toml_writer_printf(writer, "%s = [", key);
}

int toml_writer_pop_value_array(TOML_Writer *writer)
{
    if (!writer->is_writing_value_array)
    {
        printf("toml_writer: cannot pop value array, we are not in one!\n");
        return 0;
    }
    writer->is_writing_value_array = 0;
    writer->buffer_len -= 2;
    toml_writer_putc(writer, ']');
    toml_writer_putc(writer, '\n');

    return 1;
}

int toml_writer_push_string(TOML_Writer *writer, char *key, char *string)
{
    if (writer->is_writing_value_array)
    {
        toml_writer_printf(writer, "\"%s\", ", string);
        return 1;
    }

    toml_writer_prepare(writer);
    toml_writer_indent(writer);
    toml_writer_printf(writer, "%s = \"%s\"\n", key, string);

    return 1;
}

int toml_writer_push_integer(TOML_Writer *writer, char *key, int64_t integer)
{
    if (writer->is_writing_value_array)
    {
        toml_writer_printf(writer, "%d, ", integer);
        return 1;
    }

    toml_writer_prepare(writer);
    toml_writer_indent(writer);
    toml_writer_printf(writer, "%s = %d\n", key, integer);

    return 1;
}

int toml_writer_push_boolean(TOML_Writer *writer, char *key, int boolean)
{
    if (writer->is_writing_value_array)
    {
        toml_writer_printf(writer, "%s, ", boolean ? "true" : "false");
        return 1;
    }

    toml_writer_prepare(writer);
    toml_writer_indent(writer);
    toml_writer_printf(writer, "%s = %s\n", key, boolean ? "true" : "false");

    return 1;
}