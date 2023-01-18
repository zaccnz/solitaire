#include "io/tex/textures.h"

#include "io/tex/texturepack.h"
#include "util/util.h"

#include <stdlib.h>
#include <string.h>

Textures *textures_find(TexturePack *pack, const char *textures_name)
{
    if (!pack->textures)
    {
        return NULL;
    }

    for (int i = 0; i < pack->textures_count; i++)
    {
        if (!strcmp(textures_name, pack->textures[i]->name))
        {
            return pack->textures[i];
        }
    }

    return NULL;
}

Textures *textures_find_or_create(TexturePack *pack, const char *textures_name, int *created)
{
    if (created)
    {
        *created = 0;
    }

    Textures *textures = textures_find(pack, textures_name);

    if (textures)
    {
        return textures;
    }

    if (!pack->textures)
    {
        pack->textures = malloc(sizeof(Textures *) * 3);
        memset(pack->textures, 0, sizeof(Textures *) * 3);
        pack->textures_size = 3;
    }
    else if (pack->textures_count >= pack->textures_size)
    {
        int new_size = pack->textures_size * 2;
        pack->textures = realloc(pack->textures, sizeof(Textures *) * new_size);
        memset(pack->textures + pack->textures_size, 0, sizeof(Textures *) * pack->textures_size);
        pack->textures_size = new_size;
    }

    textures = malloc(sizeof(Textures));
    memset(textures, 0, sizeof(Textures));
    textures->name = textures_name;

    pack->textures[pack->textures_count] = textures;
    pack->textures_count++;

    if (textures && created)
    {
        *created = 1;
    }

    return textures;
}

int textures_fill_with_default(TexturePack *pack)
{
    Textures *default_textures = NULL;
    for (int i = 0; i < pack->textures_count; i++)
    {
        if (!strcmp(pack->textures[i]->name, "Default"))
        {
            default_textures = pack->textures[i];
            break;
        }
    }
    if (default_textures == NULL)
    {
        printf("textures: pack missing default textures\n");
        return 0;
    }
    for (int i = 0; i < MAX_CARDS; i++)
    {
        if (default_textures->cards[i].id <= 0)
        {
            printf("textures: pack missing default texture for face card %d\n", i);
            return 0;
        }
    }

    for (int i = 0; i < pack->textures_count; i++)
    {
        if (!strcmp(pack->textures[i]->name, "Default"))
        {
            continue;
        }
        if (!(pack->textures[i]->type & TEXTURE_CARDS))
        {
            continue;
        }
        Texture *card_textures = pack->textures[i]->cards;
        for (int j = 0; j < MAX_CARDS; j++)
        {
            if (card_textures[j].width == 0 && card_textures[j].height == 0)
            {
                card_textures[j] = default_textures->cards[j];
            }
        }
    }

    return 1;
}

void textures_free(Textures *textures)
{
    if (textures->type & TEXTURE_BACKGROUNDS)
    {
        UnloadTexture(textures->background);
    }
    if (textures->type & TEXTURE_BACKS)
    {
        UnloadTexture(textures->card_back);
    }
    if (textures->type & TEXTURE_CARDS)
    {
        for (int i = 0; i < MAX_CARDS; i++)
        {
            UnloadTexture(textures->cards[i]);
        }
    }

    free(textures->name);
    free(textures);
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

int textures_from_directory(Textures *textures, char *path, Suit suit, Value value, toml_array_t *files)
{
    char full_path[2048];

    for (int i = 0; i < toml_array_nelem(files); i++)
    {
        toml_datum_t value_toml = toml_string_at(files, i);
        if (!value_toml.ok)
        {
            printf("textures: failed to read files: %d is not a string\n", i);
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

        if (!texture_from_path(full_path, &textures->cards[card]))
        {
            return 0;
        }

        free(value_toml.u.s);
    }

    return 1;
}