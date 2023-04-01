#include "io/tex/texturepack.h"

#include "util/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <toml.h>

const char *BACKGROUND_TYPE_NAMES[BACKGROUND_MAX] = {
    NULL,
    "colour",
    "cover",
    "stretch",
    "tiled",
};

const char *DEFAULT_SUIT_NAMES[SUIT_MAX] = {
    "Clubs",
    "Hearts",
    "Spades",
    "Diamonds",
};

const char *DEFAULT_VALUE_NAMES[VALUE_MAX] = {
    "A",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "J",
    "Q",
    "K",
};

const char *DEFAULT_BACK_ASSET_NAMES[3] = {
    "Blue",
    "Red",
    "Green",
};

const char *DEFAULT_BACK_NAMES[3] = {
    "blue1",
    "green1",
    "red1",
};

int pack_load_default_assets(TexturePack *pack)
{
    char path[256];
    memset(path, 0, 256);

    pack->assets = malloc(sizeof(Assets *) * 4);
    pack->assets_count = 4;
    pack->assets_size = 4;

    pack->assets[0] = malloc(sizeof(Assets));
    pack->assets[0]->type = ASSET_CARDS;
    pack->assets[0]->name = strdup("Default");

    for (int i = 0; i < SUIT_MAX; i++)
    {
        for (int j = 0; j < VALUE_MAX; j++)
        {
            snprintf(path, 256, "res/tex/Cards/card%s%s.png", DEFAULT_SUIT_NAMES[i], DEFAULT_VALUE_NAMES[j]);
            pack->assets[0]->cards[i * VALUE_MAX + j] = LoadTexture(path);
        }
    }

    for (int i = 0; i < 3; i++)
    {
        pack->assets[i + 1] = malloc(sizeof(Assets));
        Assets *assets = pack->assets[i + 1];
        assets->type = ASSET_BACKS;
        assets->name = strdup(DEFAULT_BACK_ASSET_NAMES[i]);
        snprintf(path, 256, "res/tex/Cards/cardBack_%s.png", DEFAULT_BACK_NAMES[i]);
        assets->card_back = LoadTexture(path);
    }

    return 1;
}

int pack_load_default(TexturePack *pack)
{
    pack->name = strdup("Internal");
    pack->author = strdup("Kenney");
    pack->path = strdup("res/tex");

    pack->card_vertical_spacing = 0.3f;

    pack->spritesheets = 0;
    pack->spritesheets_count = 0;

    pack_load_default_assets(pack);

    return 1;
}

int pack_load_metadata(TexturePack *pack, toml_table_t *meta)
{
    toml_datum_t name = toml_string_in(meta, "name");
    if (!name.ok)
    {
        printf("pack: cannot read meta.name\n");
        return 0;
    }

    toml_datum_t author = toml_string_in(meta, "author");
    if (!author.ok)
    {
        printf("pack: cannot read meta.author\n");
        return 0;
    }

    toml_datum_t card_vertical_spacing = toml_double_in(meta, "card_vertical_spacing");
    if (!card_vertical_spacing.ok)
    {
        printf("pack: cannot read meta.card_vertical_spacing\n");
        return 0;
    }

    pack->name = name.u.s;
    pack->author = author.u.s;
    pack->card_vertical_spacing = card_vertical_spacing.u.d;

    return 1;
}

BackgroundType pack_get_background_type(char *type)
{
    for (int i = 1; i < BACKGROUND_MAX; i++)
    {
        if (!strcmp(type, BACKGROUND_TYPE_NAMES[i]))
        {
            return i;
        }
    }

    return BACKGROUND_NONE;
}

int pack_load_backgrounds(TexturePack *pack, toml_array_t *backgrounds)
{
    int subtables = toml_array_nelem(backgrounds);
    for (int i = 0; i < subtables; i++)
    {
        toml_table_t *table = toml_table_at(backgrounds, i);
        if (!table)
        {
            printf("could not read background table %d\n", i);
            continue;
        }

        toml_datum_t name = toml_string_in(table, "name");
        if (!name.ok)
        {
            printf("pack %s background %d missing name\n", pack->name, i);
            continue;
        }

        int created;
        Assets *assets = assets_find_or_create(pack, name.u.s, &created);
        if (!created)
        {
            free(name.u.s);
        }
        assets->type |= ASSET_BACKGROUNDS;

        toml_datum_t type = toml_string_in(table, "type");
        if (!type.ok)
        {
            printf("pack %s background %d missing type\n", pack->name, i);
            continue;
        }
        assets->background.type = pack_get_background_type(type.u.s);
        free(type.u.s);

        toml_datum_t placeholder = toml_string_in(table, "placeholder");
        if (!placeholder.ok)
        {
            printf("pack %s background %d missing placeholder colour\n", pack->name, i);
            continue;
        }
        assets->background.placeholder = string_to_colour(placeholder.u.s);
        free(placeholder.u.s);

        if (assets->background.type == BACKGROUND_COLOUR)
        {
            toml_datum_t colour = toml_string_in(table, "colour");
            if (!colour.ok)
            {
                printf("pack %s background %d missing colour\n", pack->name, i);
                continue;
            }
            assets->background.colour = string_to_colour(colour.u.s);
            free(colour.u.s);
        }
        else
        {
            toml_datum_t texture = toml_string_in(table, "texture");
            if (!texture.ok)
            {
                printf("pack %s background %s missing texture\n", pack->name, name.u.s);
                continue;
            }

            texture_from_path(texture.u.s, &assets->background.texture);
            free(texture.u.s);
        }
    }
    return 1;
}

int pack_load_card(TexturePack *pack, toml_table_t *card, const char *key)
{
    toml_datum_t name = toml_string_in(card, "name");
    if (!name.ok)
    {
        printf("pack %s card defintion %s missing name\n", pack->name, key);
        return 0;
    }

    int created = 0;
    Assets *assets = assets_find_or_create(pack, name.u.s, &created);
    if (!created)
    {
        free(name.u.s);
    }

    Spritesheet *spritesheet = NULL;
    char *texture = NULL;
    toml_datum_t texture_toml = toml_string_in(card, "texture");
    if (texture_toml.ok)
    {
        texture = texture_toml.u.s;
    }
    toml_datum_t spritesheet_toml = toml_string_in(card, "spritesheet");
    if (spritesheet_toml.ok)
    {
        spritesheet = pack_get_spritesheet(pack, spritesheet_toml.u.s);
        free(spritesheet_toml.u.s);
        if (!spritesheet)
        {
            printf("pack %s card definition %s invalid spritesheet %s\n",
                   pack->name, key, spritesheet_toml.u.s);
            return 0;
        }

        if (texture)
        {
            printf("pack %s card defintion %s defines both a texture and spritesheet, which is invalid\n", pack->name, key);
            free(texture);
            return 0;
        }
    }

    Texture *out = NULL;
    if (!strcmp(key, "back"))
    {
        out = &assets->card_back;
        assets->type |= ASSET_BACKS;
    }
    else
    {
        int index = key_get_index(key);
        if (index == -1)
        {
            printf("card definition invalid key %s\n", key);
            return 0;
        }
        out = &assets->cards[index];
        assets->type |= ASSET_CARDS;
    }

    int result = 0;
    if (spritesheet)
    {
        toml_datum_t row = toml_int_in(card, "row");
        if (!row.ok)
        {
            printf("card definition %s missing row\n", key);
            return 0;
        }
        toml_datum_t column = toml_int_in(card, "column");
        if (!column.ok)
        {
            printf("card definition %s missing column\n", key);
            return 0;
        }

        result = spritesheet_get_texture(spritesheet, pack, out, row.u.i, column.u.i);
    }
    else if (texture)
    {
        result = texture_from_path(texture, out);
        free(texture);
    }
    else
    {
        printf("failed to load card %s texture or spritesheet\n", key);
    }
    return result;
}

int pack_load_suit_or_values(TexturePack *pack, toml_table_t *card, Suit suit, Value value)
{
    toml_datum_t name = toml_string_in(card, "name");
    if (!name.ok)
    {
        printf("pack %s cards defintion %s missing name\n", pack->name, toml_table_key(card));
        return 0;
    }

    int created = 0;
    Assets *assets = assets_find_or_create(pack, name.u.s, &created);
    if (!created)
    {
        free(name.u.s);
    }
    assets->type |= ASSET_CARDS;

    toml_array_t *files = toml_array_in(card, "files");
    if (files)
    {
        int length = suit == SUIT_MAX ? SUIT_MAX : VALUE_MAX;
        if (toml_array_nelem(files) != length)
        {
            printf("cards definition %s files should have %d values, not %d\n",
                   toml_table_key(card), length, toml_array_nelem(files));
            return 0;
        }
        toml_datum_t path = toml_string_in(card, "path");
        if (!path.ok)
        {
            path.u.s = malloc(sizeof(char));
            path.u.s[0] = 0;
        }

        if (!assets_cards_from_directory(assets, path.u.s, suit, value, files))
        {
            printf("in pack %s\n", toml_table_key(card));
            free(path.u.s);
            return 0;
        }

        free(path.u.s);
        return 1;
    }

    toml_datum_t spritesheet = toml_string_in(card, "spritesheet");
    if (!spritesheet.ok)
    {
        printf("cards definition %s missing spritesheet\n", toml_table_key(card));
        return 0;
    }
    Spritesheet *sheet = pack_get_spritesheet(pack, spritesheet.u.s);
    if (!sheet)
    {
        printf("cards definition %s invalid spritesheet %s\n", toml_table_key(card), spritesheet.u.s);
        return 0;
    }
    free(spritesheet.u.s);

    toml_datum_t row = toml_int_in(card, "row");
    toml_datum_t column = toml_int_in(card, "column");

    if (row.ok && column.ok)
    {
        printf("cards definition %s cannot define both row and column\n", toml_table_key(card));
        return 0;
    }

    toml_array_t *arr = toml_array_in(card, "cards");
    if (!arr)
    {
        printf("cards definition %s missing cards\n", toml_table_key(card));
        return 0;
    }

    int size = toml_array_nelem(arr);

    int index = 0;
    int stride = 1;

    if (row.ok)
    {
        if (row.u.i < 0 || row.u.i >= sheet->rows)
        {
            printf("cards definition %s spritesheet has no row %lld\n", toml_table_key(card), row.u.i);
            return 0;
        }
        if (size != sheet->cols)
        {
            printf("cards definition %s spritesheet has %d columns, cards has %d entries\n",
                   toml_table_key(card), sheet->cols, size);
            return 0;
        }
        index = row.u.i * sheet->cols;
        stride = 1;
    }

    if (column.ok)
    {
        if (column.u.i < 0 || column.u.i >= sheet->cols)
        {
            printf("cards definition %s spritesheet has no column %lld\n", toml_table_key(card), column.u.i);
            return 0;
        }
        if (size != sheet->rows)
        {
            printf("cards definition %s spritesheet has %d rows, cards has %d entries\n", toml_table_key(card), sheet->rows, size);
            return 0;
        }
        index = column.u.i;
        stride = sheet->cols;
    }

    for (int i = 0; i < size; i++)
    {
        toml_datum_t value_toml = toml_string_at(arr, i);
        if (!value_toml.ok)
        {
            printf("cards definition %s cards must be string at index %d\n", toml_table_key(card), i);
            return 0;
        }
        Suit card_suit;
        Value card_value;
        if (value == VALUE_MAX)
        {
            card_value = value_get_index(value_toml.u.s);
            if (card_value == VALUE_MAX)
            {
                index += stride;
                continue;
            }
        }
        else
        {
            card_value = value;
        }
        if (suit == SUIT_MAX)
        {
            card_suit = suit_get_index(value_toml.u.s);
            if (card_suit == SUIT_MAX)
            {
                index += stride;
                continue;
            }
        }
        else
        {
            card_suit = suit;
        }
        Texture *tex = &assets->cards[card_suit * VALUE_MAX + card_value];
        free(value_toml.u.s);

        spritesheet_get_texture_index(sheet, pack, tex, index);

        index += stride;
    }

    return 1;
}

int pack_load_cards(TexturePack *pack, toml_table_t *cards)
{
    int subtables = toml_table_narr(cards);
    for (int i = 0; i < subtables; i++)
    {
        const char *key = toml_key_in(cards, i);
        toml_array_t *arr = toml_array_in(cards, key);
        if (!arr)
        {
            printf("[cards] must contain arrays\n");
            continue;
        }

        for (int j = 0; j < toml_array_nelem(arr); j++)
        {
            toml_table_t *table = toml_table_at(arr, j);
            if (!table)
            {
                printf("[cards] must contain arrays of tables\n");
                continue;
            }

            Suit suit = suit_get_index(key);
            if (suit != SUIT_MAX)
            {
                pack_load_suit_or_values(pack, table, suit, VALUE_MAX);
                continue;
            }

            Value value = values_get_index(key);
            if (value != VALUE_MAX)
            {
                pack_load_suit_or_values(pack, table, SUIT_MAX, value);
                continue;
            }

            pack_load_card(pack, table, key);
        }
    }
    return 1;
}

int pack_load(const char *path, TexturePack *pack)
{

    int path_len = strlen(path) + 1;
    pack->path = malloc(path_len * sizeof(char));
    snprintf(pack->path, path_len, "%s", path);

    pack->spritesheets = NULL;
    pack->spritesheets_count = 0;
    pack->assets = NULL;
    pack->assets_count = 0;
    pack->assets_size = 0;

    char errbuf[200];

    char *contents = physfs_read_to_mem("textures.toml", NULL);

    if (!contents)
    {
        printf("pack: failed to read file %s\n", path);
        return 0;
    }

    toml_table_t *pack_toml = toml_parse(contents, errbuf, sizeof(errbuf));

    UnloadFileText(contents);

    if (!pack_toml)
    {
        printf("pack: cannot parse - %s\n", errbuf);
        return 0;
    }

    toml_table_t *meta = toml_table_in(pack_toml, "meta");
    if (!meta)
    {
        printf("pack %s missing metadata\n", path);
        return 0;
    }

    if (!pack_load_metadata(pack, meta))
    {
        printf("pack %s failed to load metadata\n", path);
        return 0;
    }

    toml_table_t *licence = toml_table_in(pack_toml, "licence");
    if (licence)
    {
        licence_load(&pack->licence, licence);
    }

    toml_array_t *spritesheets = toml_array_in(pack_toml, "spritesheets");
    if (spritesheets && !spritesheet_load_all(spritesheets, pack))
    {
        printf("pack %s failed to load spritesheets\n", path);
        return 0;
    }

    toml_array_t *backgrounds = toml_array_in(pack_toml, "backgrounds");
    if (backgrounds && !pack_load_backgrounds(pack, backgrounds))
    {
        printf("pack %s failed to load backgrounds\n", path);
        return 0;
    }

    toml_table_t *cards = toml_table_in(pack_toml, "cards");
    if (cards && !pack_load_cards(pack, cards))
    {
        printf("pack %s failed to load cards\n", path);
        return 0;
    }

    assets_cards_fill_with_default(pack);

    toml_free(pack_toml);

    printf("==== %s ====\n", pack->path);
    printf("Texture Pack %s (%s) Loaded\n", pack->name, pack->author);
    printf("  %d assets\n", pack->assets_count);
    for (int i = 0; i < pack->assets_count; i++)
    {
        Assets *assets = pack->assets[i];
        printf("  %s %s (background %d, backs %d, cards %d)\n", pack->name, assets->name,
               assets->type & ASSET_BACKGROUNDS && 1,
               assets->type & ASSET_BACKS && 1,
               assets->type & ASSET_CARDS && 1);
    }
    printf("  %d spritesheets\n", pack->spritesheets_count);
    for (int i = 0; i < pack->spritesheets_count; i++)
    {
        Spritesheet *spritesheet = &pack->spritesheets[i];
        printf("  %s (rows %d, cols %d)\n", spritesheet->name, spritesheet->rows, spritesheet->cols);
    }
    printf("====\n");

    return 1;
}

int pack_free(TexturePack *pack)
{
    for (int i = 0; i < pack->spritesheets_count; i++)
    {
        spritesheet_free(&pack->spritesheets[i]);
    }

    for (int i = 0; i < pack->assets_count; i++)
    {
        assets_free(pack->assets[i]);
    }

    if (pack->licence.name)
    {
        licence_free(&pack->licence);
    }

    free(pack->name);
    free(pack->author);
    free(pack->path);
    if (pack->spritesheets)
    {
        free(pack->spritesheets);
    }
    if (pack->assets)
    {
        free(pack->assets);
    }

    return 1;
}

Spritesheet *pack_get_spritesheet(TexturePack *pack, const char *spritesheet)
{
    for (int i = 0; i < pack->spritesheets_count; i++)
    {
        if (!strcmp(spritesheet, pack->spritesheets[i].name))
        {
            return &pack->spritesheets[i];
        }
    }

    return NULL;
}

char **pack_get_asset_names(TexturePack *pack, int *count)
{
    char **names = malloc(sizeof(char *) * pack->assets_count);
    for (int i = 0; i < pack->assets_count; i++)
    {
        names[i] = pack->assets[i]->name;
    }
    *count = pack->assets_count;
    return names;
}