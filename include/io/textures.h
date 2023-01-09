#pragma once

#include "solitaire.h"

#include <raylib.h>

typedef struct TexturePack TexturePack;

typedef enum TEXTURETYPE
{
    TEXTURE_BACKGROUNDS = 1 << 0,
    TEXTURE_BACKS = 1 << 1,
    TEXTURE_CARDS = 1 << 2,
} TextureType;

typedef struct Textures
{
    TextureType type;
    char *name;

    Texture background;
    Texture card_back;
    Texture cards[MAX_CARDS];
} Textures;

Textures *textures_find(TexturePack *pack, const char *textures_name);
Textures *textures_find_or_create(TexturePack *pack, const char *textures_name, int *created);
int textures_fill_with_default(TexturePack *pack);

int texture_from_path(char *pack_path, char *path, Texture *out);