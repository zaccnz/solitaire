#pragma once

#include "solitaire.h"

#include <raylib.h>
#include <toml.h>

typedef struct TexturePack TexturePack;

typedef enum ASSETTYPE
{
    ASSET_NONE,
    ASSET_BACKGROUNDS = 1 << 0,
    ASSET_BACKS = 1 << 1,
    ASSET_CARDS = 1 << 2,
} AssetType;

typedef enum BACKGROUNDTYPE
{
    BACKGROUND_NONE = 0,
    BACKGROUND_COLOUR,
    BACKGROUND_COVER,
    BACKGROUND_STRETCH,
    BACKGROUND_TILE,
    BACKGROUND_MAX,
} BackgroundType;

typedef struct Assets
{
    AssetType type;
    char *name;

    struct
    {
        BackgroundType type;
        Texture texture;
        Color colour;
        Color placeholder;
    } background;

    Texture card_back;
    Texture cards[MAX_CARDS];
} Assets;

Assets *assets_find(TexturePack *pack, const char *assets_name);
Assets *assets_find_or_create(TexturePack *pack, char *assets_name, int *created);
void assets_free(Assets *assets);

int assets_cards_fill_with_default(TexturePack *pack);
int assets_cards_from_directory(Assets *assets, char *path, Suit suit, Value value, toml_array_t *files);

int texture_from_path(char *path, Texture *out);