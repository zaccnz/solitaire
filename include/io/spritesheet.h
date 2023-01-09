#pragma once

#include <raylib.h>
#include <toml.h>

typedef struct TexturePack TexturePack;

typedef struct Spritesheet
{
    char *name;
    Image image;
    int rows, cols;
    int border_top, border_bottom, border_left, border_right;
    int gap_x, gap_y;
} Spritesheet;

int spritesheet_load_all(toml_array_t *spritesheets, TexturePack *pack);
int spritesheet_validate(Spritesheet *spritesheet, TexturePack *pack);
void spritesheet_free(Spritesheet *spritesheet);

int spritesheet_get_texture(Spritesheet *spritesheet, TexturePack *pack, Texture *out, int row, int col);
int spritesheet_get_texture_index(Spritesheet *spritesheet, TexturePack *pack, Texture *out, int index);