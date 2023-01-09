#include "io/textures.h"

#include "io/texturepack.h"

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

int texture_from_path(char *pack_path, char *path, Texture *out)
{
    char texture_path[2048];
    snprintf(texture_path, 2048, "%s/%s", pack_path, path);

    *out = LoadTexture(texture_path);
    if (out->id <= 0)
    {
        printf("texture %s (%s) invalid\n", path, texture_path);
        return 0;
    }
    return 1;
}