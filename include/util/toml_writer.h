/*
 * a quick toml serializing library
 *
 * supports:
 *   - booleans
 *   - integers
 *   - strings
 *   - keys (with children)
 *   - value arrays
 *   - object arrays
 *
 * does not support
 *   - timestamp
 *   - double
 *   - nested arrays
 *   - automatically enquote keys
 *   - (most other features that arent used in config.toml or leaderboard.toml)
 */
#pragma once

#include <stdint.h>

typedef struct TOML_Writer_Key
{
    struct TOML_Writer_Key *parent;
    int parent_count;
    char *name;
    int is_array;
} TOML_Writer_Key;

typedef struct TOML_Writer
{
    char *buffer;
    int buffer_len;
    int buffer_size;

    int do_whitespace;
    int key_do_whitespace;
    int is_writing_value_array;

    TOML_Writer_Key *current;
} TOML_Writer;

TOML_Writer *toml_writer_new();
void toml_writer_free(TOML_Writer *writer);

int toml_writer_save(TOML_Writer *writer, char *filename);

int toml_writer_push_key(TOML_Writer *writer, char *key, int array);
int toml_writer_pop_key(TOML_Writer *writer);
int toml_writer_push_value_array(TOML_Writer *writer, char *key);
int toml_writer_pop_value_array(TOML_Writer *writer);
int toml_writer_push_string(TOML_Writer *writer, char *key, char *string);
int toml_writer_push_integer(TOML_Writer *writer, char *key, int64_t integer);
int toml_writer_push_boolean(TOML_Writer *writer, char *key, int boolean);