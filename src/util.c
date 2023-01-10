#include "util.h"

#include "solitaire.h"

#include <physfs.h>
#include <stdlib.h>

int ntlen(void **array)
{
    int i = 0;
    while (array[i] != NULL)
    {
        i++;
    }
    return i;
}

const char *SUITS[] = {
    "clubs",
    "hearts",
    "spades",
    "diamonds",
};

const char *VALUE[] = {
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
        printf("%s is not a valid card\n", key);
        return -1;
    }
    int split = chr - key;
    int len = strlen(key);

    if (split == 0 || split >= 31 || len - split >= 31 || len - split == 0)
    {
        printf("%s is not a valid card\n", key);
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
        printf("%s is not a valid suit\n", suit_str);
        return -1;
    }

    if (value == VALUE_MAX)
    {
        printf("%s is not a valid value\n", value_str);
        return -1;
    }

    return suit * VALUE_MAX + value;
}

char *physfs_read_to_mem(const char *path, int *size)
{
    PHYSFS_File *file = PHYSFS_openRead(path);
    if (!file)
    {
        printf("failed to open file %s for reading: %s\n", PHYSFS_getLastError(), path);
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
        printf("failed to allocate space (%d bytes) for file %s\n", *size, path);
        return NULL;
    }

    PHYSFS_readBytes(file, contents, size ? *size : len);

    return contents;
}