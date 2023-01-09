#include "io/pacman.h"

#include "gfx/cards.h"
#include "gfx/layout.h"

#include <stdlib.h>
#include <string.h>

TexturePack *packs = NULL;
int pack_count;

// called cards2 becauses cards get set to NULL at some point.
// probably by a different c file.  sorry code overlords....
typedef struct CurrentPack
{
    TexturePack *pack;
    Textures *textures;
    PackPointer pointer;
} CurrentPack;

CurrentPack pack_background = {0}, pack_backs = {0}, pack_cards = {0};

void pacman_reload_packs()
{
    if (packs)
    {
        pacman_free_packs();
    }
    else
    {
        // todo: do not hardcode, load from config and have sensible error
        // handling
        strcpy(pack_background.pointer.name, "SBS 2d Poker Pack");
        strcpy(pack_background.pointer.texture_name, "Default");
        strcpy(pack_backs.pointer.name, "SBS 2d Poker Pack");
        strcpy(pack_backs.pointer.texture_name, "Red");
        strcpy(pack_cards.pointer.name, "SBS 2d Poker Pack");
        strcpy(pack_cards.pointer.texture_name, "Default");
    }

    // TODO: find texture packs by path

    packs = malloc(sizeof(TexturePack) * 3);
    pack_count = 3;

    pack_load("res/tex/SBS_2dPokerPack", &packs[0]);
    pack_load("res/tex/8bit", &packs[1]);
    pack_load("res/tex/natomarcacini", &packs[2]);

    pacman_set_current(pack_background.pointer, TEXTURE_BACKGROUNDS);
    pacman_set_current(pack_backs.pointer, TEXTURE_BACKS);
    pacman_set_current(pack_cards.pointer, TEXTURE_CARDS);
}

void pacman_free_packs()
{
    for (int i = 0; i < pack_count; i++)
    {
        pack_free(packs + i);
    }
}

PackPointer *pacman_list_packs(int *count)
{
    const TextureType ALL_PACKS = TEXTURE_BACKGROUNDS | TEXTURE_BACKS | TEXTURE_CARDS;

    *count = 0;
    int size = pack_count * 2;
    PackPointer *results = malloc(sizeof(PackPointer) * size);
    for (int i = 0; i < pack_count; i++)
    {
        TexturePack *pack = &packs[i];
        int names_count;
        char **names = pack_get_texture_names(pack, &names_count);
        if (*count + names_count >= size - 1)
        {
            while (*count + names_count >= size - 1)
            {
                size *= 2;
            }
            results = realloc(results, sizeof(PackPointer) * size);
        }
        for (int j = 0; j < names_count; j++)
        {
            char *name = names[j];
            Textures *textures = textures_find(pack, name);
            strcpy(results[*count].name, pack->name);
            strcpy(results[*count].texture_name, name);
            results[*count].type = textures->type;
            (*count)++;
        }
    }
    return results;
}

TexturePack *pacman_find_pack(char *name)
{
    for (int i = 0; i < pack_count; i++)
    {
        if (!strcmp(packs[i].name, name))
        {
            return &packs[i];
        }
    }

    printf("found no such pack %s\n", name);

    return NULL;
}

CurrentPack *pacman_current_pack(TextureType type)
{
    switch (type)
    {
    case TEXTURE_BACKGROUNDS:
    {
        return &pack_background;
    }
    case TEXTURE_BACKS:
    {
        return &pack_backs;
    }
    case TEXTURE_CARDS:
    {
        return &pack_cards;
    }
    }

    return NULL;
}

void pacman_set_current(PackPointer pointer, TextureType as)
{
    CurrentPack *target = pacman_current_pack(as);

    target->pointer = pointer;
    target->pack = pacman_find_pack(pointer.name);
    target->textures = textures_find(target->pack, pointer.texture_name);

    if (as == TEXTURE_CARDS)
    {
        layout_pack_changed();
        cards_invalidate_all();
    }
}

TexturePack *pacman_get_current(TextureType type)
{
    CurrentPack *current = pacman_current_pack(type);

    if (!current)
    {
        return NULL;
    }

    return current->pack;
}

Textures *pacman_get_current_textures(TextureType type)
{
    CurrentPack *current = pacman_current_pack(type);

    if (!current)
    {
        return NULL;
    }

    return current->textures;
}