#pragma once

#include "io/tex/spritesheet.h"
#include "io/tex/textures.h"
#include "solitaire.h"

#include <raylib.h>

#define MAX_TEXTURES 50

typedef struct TexturePack
{
    char *name;
    char *author;
    char *path;
    float card_vertical_spacing;
    Spritesheet *spritesheets;
    int spritesheets_count;
    Textures *textures[MAX_TEXTURES];
    int texture_count;
} TexturePack;

int pack_load(const char *path, TexturePack *pack);
int pack_free(TexturePack *pack);

Spritesheet *pack_get_spritesheet(TexturePack *pack, const char *spritesheet);
// note: remember to free char ** (not individual char*'s)
char **pack_get_texture_names(TexturePack *pack, int *count);