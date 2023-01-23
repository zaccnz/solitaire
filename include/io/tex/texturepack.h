#pragma once

#include "io/tex/spritesheet.h"
#include "io/tex/assets.h"
#include "solitaire.h"

#include <raylib.h>

typedef struct TexturePack
{
    char *name;
    char *author;
    char *path;
    float card_vertical_spacing;
    Spritesheet *spritesheets;
    int spritesheets_count;
    Assets **assets;
    int assets_count;
    int assets_size;
} TexturePack;

int pack_load_default(TexturePack *pack);
int pack_load(const char *path, TexturePack *pack);
int pack_free(TexturePack *pack);

Spritesheet *pack_get_spritesheet(TexturePack *pack, const char *spritesheet);
// note: remember to free char ** (not individual char*'s)
char **pack_get_asset_names(TexturePack *pack, int *count);