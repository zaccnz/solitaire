#include "io/pacman.h"

#include "gfx/cards.h"
#include "gfx/layout.h"

#include <physfs.h>
#include <raylib.h>
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

int pacman_mount_pack(char path[2048], char *file)
{
    if (IsFileExtension(file, ".zip"))
    {
        snprintf(path, 2048, "res/tex/%s", file);
        PHYSFS_mount(path, NULL, 0);

        if (!PHYSFS_exists("textures.toml"))
        {
            char **dirs = PHYSFS_enumerateFiles("");

            if (dirs[0] == NULL)
            {
                return 0;
            }

            snprintf(path, 2048, "%s/textures.toml", dirs[0]);
            int exists = PHYSFS_exists(path);

            snprintf(path, 2048, "res/tex/%s", file);

            if (!exists)
            {
                PHYSFS_unmount(path);
                PHYSFS_freeList(dirs);
                return 0;
            }

            PHYSFS_setRoot(path, dirs[0]);
            PHYSFS_freeList(dirs);
        }
    }
    else
    {
        snprintf(path, 2048, "res/tex/%s", file);

        if (!DirectoryExists(path))
        {
            return 0;
        }

        snprintf(path, 2048, "res/tex/%s/textures.toml", file);
        if (!FileExists(path))
        {
            return 0;
        }

        snprintf(path, 2048, "res/tex/%s", file);
        PHYSFS_mount(path, NULL, 0);
    }

    return 1;
}

void pacman_reload_packs()
{
    if (packs)
    {
        pacman_free_packs();
    }

    int count = 0;
    pack_count = 0;
    char **files = GetDirectoryFiles("res/tex", &count);
    packs = malloc(sizeof(TexturePack) * count);
    char path[2048];

    for (int i = 0; i < count; i++)
    {
        if (!pacman_mount_pack(path, files[i]))
        {
            continue;
        }

        if (pack_load(path, &packs[pack_count]))
        {
            pack_count++;
        }

        PHYSFS_unmount(path);

        if (pack_count >= count)
        {
            printf("more packs then file entries?\n");
            break;
        }
    }

    ClearDirectoryFiles();

    if (pack_background.pack && pack_backs.pack && pack_cards.pack)
    {
        return;
    }

    count = 0;
    PackPointer *ptr = pacman_list_packs(&count);
    for (int i = 0; i < count; i++)
    {
        if (!pack_background.pack && ptr[i].type & TEXTURE_BACKGROUNDS)
        {
            pacman_set_current(ptr[i], TEXTURE_BACKGROUNDS);
        }

        if (!pack_backs.pack && ptr[i].type & TEXTURE_BACKS)
        {
            pacman_set_current(ptr[i], TEXTURE_BACKS);
        }

        if (!pack_cards.pack && ptr[i].type & TEXTURE_CARDS)
        {
            pacman_set_current(ptr[i], TEXTURE_CARDS);
        }
    }
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