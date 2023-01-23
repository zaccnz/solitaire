/*
 * some fairly generic utility functions
 */
#pragma once

#include "solitaire.h"

#include <raylib.h>

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

int ntlen(void **array);

typedef enum SUIT Suit;
typedef enum VALUE Value;

Suit suit_get_index(const char *suit);
Value value_get_index(const char *value);
Value values_get_index(const char *values);
int key_get_index(const char *key);

char *physfs_read_to_mem(const char *file, int *size);

Color string_to_colour(char *value);

extern const char *SUITS[SUIT_MAX];
extern const char *VALUE[VALUE_MAX];