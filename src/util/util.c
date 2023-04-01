#include "util/util.h"

#include "io/config.h"

#include <physfs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int ntlen(void **array)
{
    int i = 0;
    while (array[i] != NULL)
    {
        i++;
    }
    return i;
}

const char *SUITS[SUIT_MAX] = {
    "clubs",
    "hearts",
    "spades",
    "diamonds",
};

const char *VALUE[VALUE_MAX] = {
    "A",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "J",
    "Q",
    "K",
};

const char *VALUES[] = {
    "aces",
    "twos",
    "threes",
    "fours",
    "fives",
    "sixes",
    "sevens",
    "eights",
    "nines",
    "tens",
    "jacks",
    "queens",
    "kings",
};

Suit suit_get_index(const char *suit)
{
    Suit index = SUIT_MAX;
    for (int i = 0; i < SUIT_MAX; i++)
    {
        if (!strcmp(suit, SUITS[i]))
        {
            index = i;
        }
    }

    return index;
}

Value value_get_index(const char *value)
{
    Value index = VALUE_MAX;
    for (int i = 0; i < VALUE_MAX; i++)
    {
        if (!strcmp(value, VALUE[i]))
        {
            index = i;
        }
    }

    return index;
}

Value values_get_index(const char *values)
{
    Value index = VALUE_MAX;
    for (int i = 0; i < VALUE_MAX; i++)
    {
        if (!strcmp(values, VALUES[i]))
        {
            index = i;
        }
    }

    return index;
}

int key_get_index(const char *key)
{
    char *chr = strchr(key, '_');
    if (!chr)
    {
        printf("key_get_index: %s is not a valid card\n", key);
        return -1;
    }
    int split = chr - key;
    int len = strlen(key);

    if (split == 0 || split >= 31 || len - split >= 31 || len - split == 0)
    {
        printf("key_get_index: %s is not a valid card\n", key);
        return -1;
    }

    char value_str[32] = {0};
    char suit_str[32] = {0};
    memcpy(value_str, key, split);
    memcpy(suit_str, chr + 1, len - split - 1);

    Suit suit = suit_get_index(suit_str);
    Value value = value_get_index(value_str);

    if (suit == SUIT_MAX)
    {
        printf("key_get_index: %s is not a valid suit\n", suit_str);
        return -1;
    }

    if (value == VALUE_MAX)
    {
        printf("key_get_index: %s is not a valid value\n", value_str);
        return -1;
    }

    return suit * VALUE_MAX + value;
}

char *physfs_read_to_mem(const char *path, int *size)
{
    PHYSFS_File *file = PHYSFS_openRead(path);
    if (!file)
    {
        printf("physfs_read_to_mem: failed to open file %s for reading: %s\n", path,
               PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode()));
        return NULL;
    }

    char *contents;
    int len = PHYSFS_fileLength(file);

    if (size)
    {
        contents = malloc(len);
        *size = len;
    }
    else
    {
        contents = malloc(len + 1);
        contents[len] = 0;
    }

    if (!contents)
    {
        printf("physfs_read_to_mem: failed to allocate space (%d bytes) for file %s\n", *size, path);
        return NULL;
    }

    PHYSFS_readBytes(file, contents, size ? *size : len);
    PHYSFS_close(file);

    return contents;
}

Color string_to_colour(char *value)
{
    if (value[0] == '#')
    {
        value++;
    }

    int hex = strtol(value, NULL, 16);

    if (strlen(value) == 6)
    {
        // if no alpha is given, add a byte for opacity
        hex <<= 8;
        hex |= 0xff;
    }

    return GetColor(hex);
}

void toggle_fullscreen()
{
    int was_fullscreen = IsWindowFullscreen();

    if (was_fullscreen)
    {
        SetWindowSize(config.window_size.width, config.window_size.height);
    }
    else
    {
        SetWindowSize(2560, 1440);
    }

    ToggleFullscreen();

    if (was_fullscreen)
    {
        printf("leaving fullscreen... (%d,%d)\n", config.window_size.width, config.window_size.height);
        SetWindowSize(config.window_size.width, config.window_size.height);
        SetWindowPosition(30, 30);
    }
    else
    {
        printf("entering fullscreen...\n");
        SetWindowSize(2560, 1440);
    }
}