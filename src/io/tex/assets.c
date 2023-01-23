#include "io/tex/assets.h"

#include "io/tex/texturepack.h"
#include "util/util.h"

#include <stdlib.h>
#include <string.h>

Assets *assets_find(TexturePack *pack, const char *assets_name)
{
    if (!pack->assets)
    {
        return NULL;
    }

    for (int i = 0; i < pack->assets_count; i++)
    {
        if (!strcmp(assets_name, pack->assets[i]->name))
        {
            return pack->assets[i];
        }
    }

    return NULL;
}

Assets *assets_find_or_create(TexturePack *pack, char *assets_name, int *created)
{
    if (created)
    {
        *created = 0;
    }

    Assets *assets = assets_find(pack, assets_name);

    if (assets)
    {
        return assets;
    }

    if (!pack->assets)
    {
        pack->assets = malloc(sizeof(Assets *) * 2);
        memset(pack->assets, 0, sizeof(Assets *) * 2);
        pack->assets_size = 2;
    }
    else if (pack->assets_count >= pack->assets_size)
    {
        int new_size = pack->assets_size * 2;
        pack->assets = realloc(pack->assets, sizeof(Assets *) * new_size);
        memset(pack->assets + pack->assets_size, 0, sizeof(Assets *) * pack->assets_size);
        pack->assets_size = new_size;
    }

    assets = malloc(sizeof(Assets));
    memset(assets, 0, sizeof(Assets));
    assets->name = assets_name;

    pack->assets[pack->assets_count] = assets;
    pack->assets_count++;

    if (assets && created)
    {
        *created = 1;
    }

    return assets;
}

void assets_free(Assets *assets)
{
    if (assets->type & ASSET_BACKGROUNDS && !(assets->background.type & BACKGROUND_COLOUR))
    {
        UnloadTexture(assets->background.texture);
    }
    if (assets->type & ASSET_BACKS)
    {
        UnloadTexture(assets->card_back);
    }
    if (assets->type & ASSET_CARDS)
    {
        for (int i = 0; i < MAX_CARDS; i++)
        {
            UnloadTexture(assets->cards[i]);
        }
    }

    free(assets->name);
    free(assets);
}

int assets_cards_fill_with_default(TexturePack *pack)
{
    Assets *default_assets = NULL;
    for (int i = 0; i < pack->assets_count; i++)
    {
        if (!strcmp(pack->assets[i]->name, "Default"))
        {
            default_assets = pack->assets[i];
            break;
        }
    }

    if (default_assets == NULL)
    {
        printf("assets: pack missing default assets\n");
        return 0;
    }

    for (int i = 0; i < MAX_CARDS; i++)
    {
        if (default_assets->cards[i].id <= 0)
        {
            printf("textures: pack missing default texture for face card %d\n", i);
            return 0;
        }
    }

    for (int i = 0; i < pack->assets_count; i++)
    {
        if (pack->assets[i] == default_assets)
        {
            continue;
        }
        if (!(pack->assets[i]->type & ASSET_CARDS))
        {
            continue;
        }
        Texture *card_textures = pack->assets[i]->cards;
        for (int j = 0; j < MAX_CARDS; j++)
        {
            if (card_textures[j].id <= 0)
            {
                card_textures[j] = default_assets->cards[j];
            }
        }
    }

    return 1;
}

int assets_cards_from_directory(Assets *assets, char *path, Suit suit, Value value, toml_array_t *files)
{
    char full_path[2048];

    for (int i = 0; i < toml_array_nelem(files); i++)
    {
        toml_datum_t value_toml = toml_string_at(files, i);
        if (!value_toml.ok)
        {
            printf("assets: failed to read files: %d is not a string\n", i);
            return 0;
        }

        int card = 0;

        if (suit == SUIT_MAX)
        {
            card = i * VALUE_MAX + value;
        }
        else if (value == VALUE_MAX)
        {
            card = suit * VALUE_MAX + i;
        }

        snprintf(full_path, 2048, "%s/%s", path, value_toml.u.s);

        if (!texture_from_path(full_path, &assets->cards[card]))
        {
            return 0;
        }

        free(value_toml.u.s);
    }

    return 1;
}

int texture_from_path(char *path, Texture *out)
{
    int size = 0;
    char *data = physfs_read_to_mem(path, &size);
    if (!data)
    {
        return 0;
    }

    Image image = LoadImageFromMemory(GetFileExtension(path), data, size);
    *out = LoadTextureFromImage(image);
    UnloadImage(image);
    if (out->id <= 0)
    {
        printf("textures: texture %s failed to load\n", path);
        return 0;
    }
    return 1;
}