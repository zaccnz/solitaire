#include "io/config.h"

#include "util/toml_writer.h"

#include <stdio.h>
#include <string.h>
#include <toml.h>

const Config DEFAULT_CONFIG = {
    .animations = 1,
    .fullscreen = 0,
    .sfx = 1,
    .window_size = {
        .width = 800,
        .height = 600,
    },
    .solitaire = {
        .dealthree = 1,
        .timed = 1,
    },
    .textures = {
        .background = {
            .pack = CFG_DEFAULT_PACK,
            .texture_name = CFG_DEFAULT_TEXTURE_NAME,
        },
        .backs = {
            .pack = CFG_DEFAULT_PACK,
            .texture_name = CFG_DEFAULT_TEXTURE_NAME_BACKS,
        },
        .cards = {
            .pack = CFG_DEFAULT_PACK,
            .texture_name = CFG_DEFAULT_TEXTURE_NAME,
        },
    },
    .debug = {
        .render_hitboxes = 0,
        .render_animation_list = 0,
        .seed = 0,
    },
};

Config config = {0};

int config_try_load_solitaire(toml_table_t *solitaire)
{
    toml_datum_t dealthree = toml_bool_in(solitaire, "dealthree");
    if (!dealthree.ok)
    {
        printf("config missing solitaire.dealthree\n");
        return 0;
    }
    config.solitaire.dealthree = dealthree.u.b;
    toml_datum_t timed = toml_bool_in(solitaire, "timed");
    if (!timed.ok)
    {
        printf("config missing solitaire.timed\n");
        return 0;
    }
    config.solitaire.timed = timed.u.b;
}

struct TextureStruct
{
    char *pack;
    char *texture_name;
};

int config_try_load_texture(
    toml_table_t *texture, struct TextureStruct *texture_struct)
{
    toml_datum_t pack = toml_string_in(texture, "pack");
    if (!pack.ok)
    {
        printf("config textures.%s.pack missing\n", toml_table_key(texture));
        return 0;
    }
    texture_struct->pack = pack.u.s;
    toml_datum_t texture_name = toml_string_in(texture, "texture_name");
    if (!texture_name.ok)
    {
        printf("config textures.%s.texture_name missing\n", toml_table_key(texture));
        return 0;
    }
    texture_struct->texture_name = texture_name.u.s;
}

int config_try_load_textures(toml_table_t *textures)
{
    const char *KEYS[3] = {"background", "backs", "cards"};
    const struct TextureStruct *STRUCTS[3] = {
        &config.textures.background,
        &config.textures.backs,
        &config.textures.cards,
    };

    for (int i = 0; i < 3; i++)
    {
        toml_table_t *texture = toml_table_in(textures, KEYS[i]);
        if (!texture)
        {
            printf("config missing textures.%s table\n", KEYS[i]);
            return 0;
        }
        if (!config_try_load_texture(texture, STRUCTS[i]))
        {
            return 0;
        }
    }

    return 1;
}

int config_try_load_debug(toml_table_t *debug)
{
    toml_datum_t render_hitboxes = toml_bool_in(debug, "render_hitboxes");
    if (!render_hitboxes.ok)
    {
        printf("config missing debug.render_hitboxes\n");
        return 0;
    }
    config.debug.render_hitboxes = render_hitboxes.u.b;
    toml_datum_t render_animation_list = toml_bool_in(debug, "render_animation_list");
    if (!render_animation_list.ok)
    {
        printf("config missing debug.render_animation_list\n");
        return 0;
    }
    config.debug.render_animation_list = render_animation_list.u.b;
    toml_datum_t seed = toml_int_in(debug, "seed");
    if (!seed.ok)
    {
        printf("config missing debug.seed\n");
        return 0;
    }
    config.debug.seed = seed.u.i;
    return 1;
}

int config_from_toml(toml_table_t *config_toml)
{
    toml_datum_t animations = toml_bool_in(config_toml, "animations");
    if (!animations.ok)
    {
        printf("config missing animations\n");
        return 0;
    }
    config.animations = animations.u.b;
    toml_datum_t fullscreen = toml_bool_in(config_toml, "fullscreen");
    if (!fullscreen.ok)
    {
        printf("config missing fullscreen\n");
        return 0;
    }
    config.fullscreen = fullscreen.u.b;
    toml_datum_t sfx = toml_bool_in(config_toml, "sfx");
    if (!sfx.ok)
    {
        printf("config missing sfx\n");
        return 0;
    }
    config.sfx = sfx.u.b;

    toml_array_t *window_size = toml_array_in(config_toml, "window_size");
    if (!window_size)
    {
        printf("config missing window_size array\n");
        return 0;
    }
    if (toml_array_nelem(window_size) != 2)
    {
        printf("config window_size should be an array of two integers\n");
        return 0;
    }
    toml_datum_t win_width = toml_int_at(window_size, 0);
    if (!win_width.ok)
    {
        printf("config window_size: first entry is not an integer\n");
        return 0;
    }
    config.window_size.width = win_width.u.i;
    toml_datum_t win_height = toml_int_at(window_size, 1);
    if (!win_height.ok)
    {
        printf("config window_size: second entry is not an integer\n");
        return 0;
    }
    config.window_size.height = win_height.u.i;

    toml_table_t *solitaire_toml = toml_table_in(config_toml, "solitaire");
    if (!solitaire_toml)
    {
        printf("config missing solitaire table\n");
        return 0;
    }
    if (!config_try_load_solitaire(solitaire_toml))
    {
        return 0;
    }

    toml_table_t *textures_toml = toml_table_in(config_toml, "textures");
    if (!textures_toml)
    {
        printf("config missing textures table\n");
        return 0;
    }
    if (!config_try_load_textures(textures_toml))
    {
        return 0;
    }

    toml_table_t *debug_toml = toml_table_in(config_toml, "debug");
    if (!debug_toml)
    {
        printf("config missing debug table\n");
        return 0;
    }
    if (!config_try_load_debug(debug_toml))
    {
        return 0;
    }

    return 1;
}

void config_load()
{
    config = DEFAULT_CONFIG;

    FILE *fp = fopen("res/config.toml", "r");
    if (!fp)
    {
        printf("cannot open res/config.toml - %s\n", strerror(errno));
    }

    char errbuf[200];
    toml_table_t *config_toml = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!config_toml)
    {
        printf("cannot parse - %s\n", errbuf);
    }

    config_from_toml(config_toml);

    toml_free(config_toml);
}

void config_free(Config *config)
{
    if (config->textures.background.pack != CFG_DEFAULT_PACK)
    {
        free(config->textures.background.pack);
    }
    if (config->textures.background.texture_name != CFG_DEFAULT_TEXTURE_NAME)
    {
        free(config->textures.background.texture_name);
    }

    if (config->textures.backs.pack != CFG_DEFAULT_PACK)
    {
        free(config->textures.backs.pack);
    }
    if (config->textures.backs.texture_name != CFG_DEFAULT_TEXTURE_NAME_BACKS)
    {
        free(config->textures.backs.texture_name);
    }

    if (config->textures.cards.pack != CFG_DEFAULT_PACK)
    {
        free(config->textures.cards.pack);
    }
    if (config->textures.cards.texture_name != CFG_DEFAULT_TEXTURE_NAME)
    {
        free(config->textures.cards.texture_name);
    }
}

void config_save()
{
    TOML_Writer *writer = toml_writer_new();

    toml_writer_push_boolean(writer, "animations", config.animations);
    toml_writer_push_boolean(writer, "fullscreen", config.fullscreen);
    toml_writer_push_boolean(writer, "sfx", config.sfx);
    toml_writer_push_value_array(writer, "window_size");
    toml_writer_push_integer(writer, NULL, config.window_size.width);
    toml_writer_push_integer(writer, NULL, config.window_size.height);
    toml_writer_pop_value_array(writer);

    toml_writer_push_key(writer, "solitaire", 0);
    toml_writer_push_boolean(writer, "dealthree", config.solitaire.dealthree);
    toml_writer_push_boolean(writer, "timed", config.solitaire.timed);
    toml_writer_pop_key(writer);

    toml_writer_push_key(writer, "textures", 0);
    toml_writer_push_key(writer, "background", 0);
    toml_writer_push_string(writer, "pack", config.textures.background.pack);
    toml_writer_push_string(writer, "texture_name", config.textures.background.texture_name);
    toml_writer_pop_key(writer);
    toml_writer_push_key(writer, "backs", 0);
    toml_writer_push_string(writer, "pack", config.textures.backs.pack);
    toml_writer_push_string(writer, "texture_name", config.textures.backs.texture_name);
    toml_writer_pop_key(writer);
    toml_writer_push_key(writer, "cards", 0);
    toml_writer_push_string(writer, "pack", config.textures.cards.pack);
    toml_writer_push_string(writer, "texture_name", config.textures.cards.texture_name);
    toml_writer_pop_key(writer);
    toml_writer_pop_key(writer);

    toml_writer_push_key(writer, "debug", 0);
    toml_writer_push_boolean(writer, "render_hitboxes", config.debug.render_hitboxes);
    toml_writer_push_boolean(writer, "render_animation_list", config.debug.render_animation_list);
    toml_writer_push_integer(writer, "seed", config.debug.seed);
    toml_writer_pop_key(writer);

    toml_writer_save(writer, "res/config.toml");
    toml_writer_free(writer);
}