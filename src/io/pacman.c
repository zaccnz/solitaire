#include "io/pacman.h"

#include "gfx/cards.h"
#include "gfx/layout.h"
#include "io/config.h"

#include <physfs.h>
#include <raylib.h>
#include <stdlib.h>
#include <string.h>

typedef struct CurrentPack
{
    TexturePack *pack;
    Assets *assets;
    PackPointer pointer;
    struct TextureConfig *config;
} CurrentPack;

TexturePack *packs = NULL;
int pack_count;

CurrentPack pack_background = {0}, pack_backs = {0}, pack_cards = {0};

int pacman_mount_pack(char path[2048], char *file)
{
    if (IsFileExtension(file, ".zip"))
    {
        snprintf(path, 2048, "packs/%s", file);
        PHYSFS_mount(path, NULL, 0);

        if (!PHYSFS_exists("textures.toml"))
        {
            char **dirs = PHYSFS_enumerateFiles("");

            if (dirs[0] == NULL)
            {
                PHYSFS_unmount(path);
                return 0;
            }

            snprintf(path, 2048, "%s/textures.toml", dirs[0]);
            int exists = PHYSFS_exists(path);

            snprintf(path, 2048, "packs/%s", file);

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
        snprintf(path, 2048, "packs/%s", file);

        if (!DirectoryExists(path))
        {
            return 0;
        }

        snprintf(path, 2048, "packs/%s/textures.toml", file);
        if (!FileExists(path))
        {
            return 0;
        }

        snprintf(path, 2048, "packs/%s", file);
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
    char **files = GetDirectoryFiles("packs", &count);
    packs = malloc(sizeof(TexturePack) * (count + 1));

    pack_load_default(&packs[0]);
    pack_count = 1;

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
            printf("pacman: more packs then file entries?\n");
            break;
        }
    }

    pack_background.config = &config.textures.background;
    pack_backs.config = &config.textures.backs;
    pack_cards.config = &config.textures.cards;

    ClearDirectoryFiles();

    // load from current pointers
    if (pack_background.pack && pack_backs.pack && pack_cards.pack)
    {
        pacman_set_current(pack_background.pointer, ASSET_BACKGROUNDS);
        pacman_set_current(pack_backs.pointer, ASSET_BACKS);
        pacman_set_current(pack_cards.pointer, ASSET_CARDS);

        if (pack_background.pack && pack_backs.pack && pack_cards.pack)
        {
            // load from current pointers success
            return;
        }
    }

    // load from config
    const AssetType ASSET_TYPES[3] = {
        ASSET_BACKGROUNDS,
        ASSET_BACKS,
        ASSET_CARDS,
    };
    const struct TextureConfig *STRUCTS[3] = {
        &config.textures.background,
        &config.textures.backs,
        &config.textures.cards,
    };

    for (int i = 0; i < 3; i++)
    {
        PackPointer pointer = {
            .type = ASSET_TYPES[i],
            .name = {0},
            .texture_name = {0},
        };
        strncpy(pointer.name, STRUCTS[i]->pack, 256);
        strncpy(pointer.texture_name, STRUCTS[i]->texture_name, 256);
        pacman_set_current(pointer, ASSET_TYPES[i]);
    }

    if (pack_background.pack && pack_backs.pack && pack_cards.pack)
    {
        // load from config success
        return;
    }

    // worst case: load the first possible pack
    count = 0;
    PackPointer *ptr = pacman_list_packs(&count);
    for (int i = 0; i < count; i++)
    {
        if (!pack_background.pack && ptr[i].type & ASSET_BACKGROUNDS)
        {
            pacman_set_current(ptr[i], ASSET_BACKGROUNDS);
        }

        if (!pack_backs.pack && ptr[i].type & ASSET_BACKS)
        {
            pacman_set_current(ptr[i], ASSET_BACKS);
        }

        if (!pack_cards.pack && ptr[i].type & ASSET_CARDS)
        {
            pacman_set_current(ptr[i], ASSET_CARDS);
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
            Assets *assets = assets_find(pack, name);
            strcpy(results[*count].name, pack->name);
            strcpy(results[*count].texture_name, name);
            results[*count].type = assets->type;
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

    printf("pacman: found no such pack %s\n", name);

    return NULL;
}

CurrentPack *pacman_current_pack(AssetType type)
{
    switch (type)
    {
    case ASSET_BACKGROUNDS:
    {
        return &pack_background;
    }
    case ASSET_BACKS:
    {
        return &pack_backs;
    }
    case ASSET_CARDS:
    {
        return &pack_cards;
    }
    }

    return NULL;
}

void pacman_set_current(PackPointer pointer, AssetType as)
{
    CurrentPack *target = pacman_current_pack(as);

    target->pointer = pointer;
    target->pack = pacman_find_pack(pointer.name);
    target->assets = assets_find(target->pack, pointer.texture_name);

    config_push_pack(target->config, pointer.name, pointer.texture_name);
    config_save();

    if (as == ASSET_CARDS)
    {
        layout_pack_changed();
        cards_invalidate_all();
    }
}

TexturePack *pacman_get_current(AssetType type)
{
    CurrentPack *current = pacman_current_pack(type);

    if (!current)
    {
        return NULL;
    }

    return current->pack;
}

Assets *pacman_get_current_assets(AssetType type)
{
    CurrentPack *current = pacman_current_pack(type);

    if (!current)
    {
        return NULL;
    }

    return current->assets;
}