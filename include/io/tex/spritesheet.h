#pragma once

#include <raylib.h>
#include <toml.h>

typedef struct TexturePack TexturePack;

typedef struct Spritesheet
{
    char *name;
    Image image;
    int rows, cols;
} Spritesheet;

int spritesheet_load_all(toml_array_t *spritesheets, TexturePack *pack);
int spritesheet_validate(Spritesheet *spritesheet);
void spritesheet_free(Spritesheet *spritesheet);

int spritesheet_get_texture(Spritesheet *spritesheet, TexturePack *pack, Texture *out, int row, int col);
int spritesheet_get_texture_index(Spritesheet *spritesheet, TexturePack *pack, Texture *out, int index);