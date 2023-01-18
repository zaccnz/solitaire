#include "io/tex/texturepack.h"

#include "util/util.h"

#include <stdio.h>
#include <string.h>
#include <toml.h>

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

        toml_datum_t texture = toml_string_in(table, "texture");
        if (!texture.ok)
        {
            printf("pack %s background %s missing texture\n", pack->name, name.u.s);
            continue;
        }

        int created;
        Textures *textures = textures_find_or_create(pack, name.u.s, &created);
        if (!created)
        {
            free(name.u.s);
        }
        textures->type |= TEXTURE_BACKGROUNDS;

        texture_from_path(texture.u.s, &textures->background);
        free(texture.u.s);
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
    Textures *textures = textures_find_or_create(pack, name.u.s, &created);
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
        out = &textures->card_back;
        textures->type |= TEXTURE_BACKS;
    }
    else
    {
        int index = key_get_index(key);
        if (index == -1)
        {
            printf("card definition invalid key %s\n", key);
            return 0;
        }
        out = &textures->cards[index];
        textures->type |= TEXTURE_CARDS;
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
    Textures *textures = textures_find_or_create(pack, name.u.s, &created);
    if (!created)
    {
        free(name.u.s);
    }
    textures->type |= TEXTURE_CARDS;

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

        if (!textures_from_directory(textures, path.u.s, suit, value, files))
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
            printf("cards definition %s spritesheet has no row %d\n", toml_table_key(card), row.u.i);
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
            printf("cards definition %s spritesheet has no column %d\n", toml_table_key(card), column.u.i);
            return 0;
        }
        if (size != sheet->rows)
        {
            printf("cards definition %s spritesheet has %d rows, cards has %d entries\n", sheet->rows, size);
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
        Texture *tex = &textures->cards[card_suit * VALUE_MAX + card_value];
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
    pack->textures = NULL;
    pack->textures_count = 0;
    pack->textures_size = 0;

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

    textures_fill_with_default(pack);

    toml_free(pack_toml);

    printf("==== %s ====\n", pack->path);
    printf("Texture Pack %s (%s) Loaded\n", pack->name, pack->author);
    printf("  %d textures\n", pack->textures_count);
    for (int i = 0; i < pack->textures_count; i++)
    {
        Textures *textures = pack->textures[i];
        printf("  %s %s (background %d, backs %d, cards %d)\n", pack->name, textures->name,
               textures->type & TEXTURE_BACKGROUNDS && 1,
               textures->type & TEXTURE_BACKS && 1,
               textures->type & TEXTURE_CARDS && 1);
    }
    printf("  %d spritesheets\n", pack->spritesheets_count);
    for (int i = 0; i < pack->spritesheets_count; i++)
    {
        Spritesheet *spritesheet = &pack->spritesheets[i];
        printf("  %s (rows %d, cols %d)\n", spritesheet->name, spritesheet->rows, spritesheet->cols);
    }
    printf("====\n");

    return pack;
}

int pack_free(TexturePack *pack)
{
    for (int i = 0; i < pack->spritesheets_count; i++)
    {
        spritesheet_free(&pack->spritesheets[i]);
    }

    for (int i = 0; i < pack->textures_count; i++)
    {
        textures_free(pack->textures[i]);
    }

    free(pack->name);
    free(pack->author);
    free(pack->path);
    if (pack->spritesheets)
    {
        free(pack->spritesheets);
    }
    if (pack->textures)
    {
        free(pack->textures);
    }

    return 0;
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

char **pack_get_texture_names(TexturePack *pack, int *count)
{
    char **names = malloc(sizeof(char *) * pack->textures_count);
    for (int i = 0; i < pack->textures_count; i++)
    {
        names[i] = pack->textures[i]->name;
    }
    *count = pack->textures_count;
    return names;
}