#pragma once

#include "io/texturepack.h"

typedef struct PackPointer
{
    TextureType type;
    char texture_name[256];
    char name[256];
} PackPointer;

void pacman_reload_packs();
void pacman_free_packs();

// note: must free the result
PackPointer *pacman_list_packs(int *count);

void pacman_set_current(PackPointer pointer, TextureType type);
TexturePack *pacman_get_current(TextureType type);
Textures *pacman_get_current_textures(TextureType type);

/*
TexturePack *textures_fetch_packs(int *count);
int textures_load_pack(const char *path, TexturePack *pack);
void textures_free_packs(TexturePack *packs, int count);
*/