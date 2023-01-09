#include "io/spritesheet.h"

#include "io/texturepack.h"

int spritesheet_load_dimensions(Spritesheet *sheet, toml_array_t *dimensions)
{
    int len = toml_array_nelem(dimensions);
    if (len != 2)
    {
        printf("spritesheet %s invalid dimensions: should be [rows, columns]\n", sheet->name);
        return 0;
    }

    toml_datum_t rows = toml_int_at(dimensions, 0);
    if (!rows.ok)
    {
        printf("spritesheet %s invalid dimensions: rows must be integer\n", sheet->name);
        return 0;
    }

    toml_datum_t columns = toml_int_at(dimensions, 1);
    if (!columns.ok)
    {
        printf("spritesheet %s invalid dimensions: columns must be integer\n", sheet->name);
        return 0;
    }

    sheet->rows = rows.u.i;
    sheet->cols = columns.u.i;
}

int spritesheet_load_borders(Spritesheet *sheet, toml_array_t *borders)
{
    int count = toml_array_nelem(borders);

    if (count != 2 && count != 4)
    {
        printf("spritesheet %s invalid borders: must be [x, y] or [left, top, bottom, right]\n", sheet->name);
        return 0;
    }

    int values[4] = {0};
    for (int i = 0; i < count; i++)
    {
        toml_datum_t value = toml_int_at(borders, i);
        if (!value.ok)
        {
            printf("spritesheet %s invalid borders: values (%d) must be integer\n", sheet->name, i);
            return 0;
        }

        values[i] = value.u.i;
    }

    if (count == 2)
    {
        sheet->border_left = sheet->border_right = values[0];
        sheet->border_top = sheet->border_bottom = values[1];
    }
    else
    {
        sheet->border_left = values[0];
        sheet->border_top = values[1];
        sheet->border_bottom = values[2];
        sheet->border_right = values[3];
    }
}

int spritesheet_load(Spritesheet *sheet, toml_table_t *toml, TexturePack *pack)
{
    toml_datum_t name = toml_string_in(toml, "name");
    if (!name.ok)
    {
        printf("spritesheet %d missing name\n", (sheet - pack->spritesheets) / sizeof(Spritesheet));
        return 0;
    }

    int name_len = strlen(name.u.s) + 1;
    sheet->name = malloc(sizeof(char) * name_len);
    snprintf(sheet->name, name_len, "%s", name.u.s);
    free(name.u.s);

    toml_datum_t path = toml_string_in(toml, "texture");
    if (!path.ok)
    {
        printf("spritesheet %s missing texture\n", sheet->name);
        return 0;
    }

    char spritesheet_texture_path[2048];
    snprintf(spritesheet_texture_path, 2048, "%s/%s", pack->path, path.u.s);
    free(path.u.s);

    sheet->image = LoadImage(spritesheet_texture_path);
    if (!sheet->image.data)
    {
        printf("spritesheet %s failed to load texture %s\n", sheet->name, spritesheet_texture_path);
        return 0;
    }

    toml_array_t *dimensions = toml_array_in(toml, "dimensions");
    if (dimensions && !spritesheet_load_dimensions(sheet, dimensions))
    {
        return 0;
    }

    toml_array_t *borders = toml_array_in(toml, "borders");
    if (borders && !spritesheet_load_borders(sheet, borders))
    {
        return 0;
    }

    if (!spritesheet_validate(sheet, pack))
    {
        return 0;
    }

    return 1;
}

int spritesheet_load_all(toml_array_t *spritesheets, TexturePack *pack)
{
    pack->spritesheets_count = toml_array_nelem(spritesheets);
    pack->spritesheets = malloc(sizeof(Spritesheet) * pack->spritesheets_count);
    memset(pack->spritesheets, 0, sizeof(Spritesheet) * pack->spritesheets_count);

    for (int i = 0; i < pack->spritesheets_count; i++)
    {
        Spritesheet *spritesheet = &pack->spritesheets[i];
        toml_table_t *spritesheet_toml = toml_table_at(spritesheets, i);
        if (!spritesheet_toml)
        {
            printf("spritesheet must be an array of tables!\n");
            continue;
        }

        spritesheet_load(spritesheet, spritesheet_toml, pack);
    }

    return 1;
}

int spritesheet_validate(Spritesheet *spritesheet)
{
    // todo: new spritesheet validation using borders and gap

    /*
        if (spritesheet->image.width / spritesheet->cols != pack->card_width)
        {
            printf("spritesheet %s does not have %d columns\n", spritesheet->name, spritesheet->cols);
        }

        if (spritesheet->image.height / spritesheet->rows != pack->card_height)
        {
            printf("spritesheet %s does not have %d rows\n", spritesheet->name, spritesheet->rows);
        }
    */
    return 1;
}

void spritesheet_free(Spritesheet *spritesheet)
{
    UnloadImage(spritesheet->image);
    free(spritesheet->name);
}

int spritesheet_get_texture(Spritesheet *spritesheet, TexturePack *pack, Texture *out, int row, int col)
{
    // todo: calculate using borders, gap, etc
    int width = spritesheet->image.width / spritesheet->cols;
    int height = spritesheet->image.height / spritesheet->rows;
    int x = col * width;
    int y = row * height;

    Rectangle rect = {
        .x = x,
        .y = y,
        .width = width,
        .height = height,
    };

    Image card = ImageFromImage(spritesheet->image, rect);

    *out = LoadTextureFromImage(card);
    UnloadImage(card);

    return 1;
}

int spritesheet_get_texture_index(Spritesheet *spritesheet, TexturePack *pack, Texture *out, int index)
{
    if (index < 0 || index >= spritesheet->rows * spritesheet->cols)
    {
        printf("invalid index %d\n", index);
        return 0;
    }

    int row = index / spritesheet->cols;
    int col = index % spritesheet->cols;

    return spritesheet_get_texture(spritesheet, pack, out, row, col);
}