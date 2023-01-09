/*
 * some fairly generic utility functions
 */
#pragma once

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