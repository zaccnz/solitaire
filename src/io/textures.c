#include "io/textures.h"

#include "io/texturepack.h"
#include "util/util.h"

Textures *textures_find(TexturePack *pack, const char *textures_name)
{

    for (int i = 0; i < pack->texture_count; i++)
    {
        if (!strcmp(textures_name, pack->textures[i]->name))
        {
            return pack->textures[i];
        }
    }

    return NULL;
}

Textures *textures_find_or_create(TexturePack *pack, const char *textures_name, int *create)
{
    if (create)
    {
        *create = 0;
    }

    Textures *textures = textures_find(pack, textures_name);

    if (textures)
    {
        return textures;
    }

    textures = malloc(sizeof(Textures));
    memset(textures, 0, sizeof(Textures));
    textures->name = textures_name;

    pack->textures[pack->texture_count] = textures;
    pack->texture_count++;

    if (textures && create)
    {
        *create = 1;
    }

    return textures;
}

int textures_fill_with_default(TexturePack *pack)
{
    Textures *default_textures = NULL;
    for (int i = 0; i < pack->texture_count; i++)
    {
        if (!strcmp(pack->textures[i]->name, "Default"))
        {
            default_textures = pack->textures[i];
            break;
        }
    }
    if (default_textures == NULL)
    {
        printf("pack missing default textures\n");
        return 0;
    }
    for (int i = 0; i < MAX_CARDS; i++)
    {
        if (default_textures->cards[i].id <= 0)
        {
            printf("pack missing default texture for face card %d\n", i);
            return 0;
        }
    }

    for (int i = 0; i < pack->texture_count; i++)
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
        printf("texture %s invalid\n", path);
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
            printf("failed to read files: %d is not a string\n", i);
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