#include "scenes/scene.h"

#include "io/config.h"
#include "io/licences.h"
#include "io/pacman.h"
#include "util/util.h"

#include <raylib.h>
#include <raylib-nuklear.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

PackPointer *pointers = NULL;

struct DropdownData
{
    int last;
    char *avail[256];
    PackPointer *pointers[256];
    int length;
} background_data, backs_data, cards_data;

int settings_was_opened = 0;
int settings_panel_width = 0;

typedef enum SETTINGSPAGES
{
    PAGE_GAME = 0,
    PAGE_TEXTURE_PACKS,
    PAGE_DEBUG,
    PAGE_OSS,
    PAGE_MAX,
} SettingsPages;

SettingsPages settings_page = PAGE_GAME;

int data_add_pointer(struct DropdownData *data, PackPointer *ptr)
{
    data->avail[data->length] = malloc(256);
    if (!strcmp(ptr->texture_name, "Default"))
    {
        memcpy(data->avail[data->length], ptr->name, 256);
    }
    else
    {
        snprintf(data->avail[data->length], 256, "%s - %s", ptr->name, ptr->texture_name);
    }
    int index = data->length;
    data->pointers[data->length++] = ptr;
    return index;
}

void settings_refresh_dropdown()
{
    if (pointers)
    {
        free(pointers);
    }
    int count = 0;
    pointers = pacman_list_packs(&count);
    background_data.length = 0;
    backs_data.length = 0;
    cards_data.length = 0;

    TexturePack *backgrounds_current = pacman_get_current(TEXTURE_BACKGROUNDS);
    Textures *backgrounds_current_textures = pacman_get_current_textures(TEXTURE_BACKGROUNDS);
    TexturePack *backs_current = pacman_get_current(TEXTURE_BACKS);
    Textures *backs_current_textures = pacman_get_current_textures(TEXTURE_BACKS);
    TexturePack *cards_current = pacman_get_current(TEXTURE_CARDS);
    Textures *cards_current_textures = pacman_get_current_textures(TEXTURE_CARDS);

    for (int i = 0; i < min(count, 256); i++)
    {
        PackPointer *ptr = &pointers[i];
        if (ptr->type & TEXTURE_BACKGROUNDS)
        {
            int index = data_add_pointer(&background_data, ptr);
            if (!strcmp(backgrounds_current->name, ptr->name) &&
                !strcmp(backgrounds_current_textures->name, ptr->texture_name))
            {
                background_data.last = index;
            }
        }
        if (ptr->type & TEXTURE_BACKS)
        {
            int index = data_add_pointer(&backs_data, ptr);
            if (!strcmp(backs_current->name, ptr->name) &&
                !strcmp(backs_current_textures->name, ptr->texture_name))
            {
                backs_data.last = index;
            }
        }
        if (ptr->type & TEXTURE_CARDS)
        {
            int index = data_add_pointer(&cards_data, ptr);
            if (!strcmp(cards_current->name, ptr->name) &&
                !strcmp(cards_current_textures->name, ptr->texture_name))
            {
                cards_data.last = index;
            }
        }
    }
}

void settings_start()
{
    printf("started settings scene\n");
    settings_was_opened = 1;
    settings_page = PAGE_GAME;
    settings_refresh_dropdown();
}

void settings_stop()
{
    printf("stopped settings scene\n");
}

void settings_update(float dt, int background)
{
    if (settings_was_opened)
    {
        settings_was_opened = 0;
        return;
    }
    if (IsKeyPressed(KEY_P))
    {
        scene_pop();
    }
}

int settings_nk_draw_dropdown(struct nk_context *ctx, char *name, TextureType type, struct DropdownData *data)
{
    nk_label(ctx, name, NK_TEXT_LEFT);
    nk_layout_row_static(ctx, 25, 200, 1);
    int current = nk_combo(ctx, data->avail, data->length,
                           data->last, 25, nk_vec2(200, 200));

    if (current != data->last)
    {
        data->last = current;
        pacman_set_current(*data->pointers[current], type);
    }
}

void settings_render_game(struct nk_context *ctx)
{
    static int animations = 0;
    nk_layout_row_static(ctx, 40, settings_panel_width, 1);
    nk_label(ctx, "Game Settings", NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED);
    nk_label(ctx, "Solitaire", NK_TEXT_ALIGN_LEFT);
    nk_layout_row_dynamic(ctx, 30, 1);
    if (nk_checkbox_label(ctx, "Deal three", &config.solitaire.dealthree))
    {
        config_save();
    }
    if (nk_checkbox_label(ctx, "Timed games", &config.solitaire.timed))
    {
        config_save();
    }
    nk_spacer(ctx);
    nk_label(ctx, "Application", NK_TEXT_ALIGN_LEFT);
    if (nk_checkbox_label(ctx, "Fullscreen", &config.fullscreen))
    {
        config_save();
    }
    if (nk_checkbox_label(ctx, "Animations", &config.animations))
    {
        config_save();
    }
    if (nk_checkbox_label(ctx, "Sfx", &config.sfx))
    {
        config_save();
    }
}

void settings_render_texture_packs(struct nk_context *ctx)
{
    nk_layout_row_dynamic(ctx, 40, 1);
    nk_label(ctx, "Texture Pack Settings", NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED);
    nk_layout_row_dynamic(ctx, 30, 1);
    if (nk_button_label(ctx, "Reload texture packs"))
    {
        pacman_reload_packs();
        settings_refresh_dropdown();
    }

    nk_layout_row_dynamic(ctx, 30, 1);
    settings_nk_draw_dropdown(ctx, "Background", TEXTURE_BACKGROUNDS, &background_data);
    settings_nk_draw_dropdown(ctx, "Card Back", TEXTURE_BACKS, &backs_data);
    settings_nk_draw_dropdown(ctx, "Cards", TEXTURE_CARDS, &cards_data);
}

void settings_render_debug(struct nk_context *ctx)
{
    static char seed_buffer[256] = {0};

    nk_layout_row_dynamic(ctx, 40, 1);
    nk_label(ctx, "Debug Settings", NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED);
    nk_layout_row_dynamic(ctx, 30, 1);
    if (nk_checkbox_label(ctx, "Card hitboxes", &config.debug.render_hitboxes))
    {
        config_save();
    }
    if (nk_checkbox_label(ctx, "Animation list", &config.debug.render_animation_list))
    {
        config_save();
    }
    nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
    nk_layout_row_push(ctx, 60);
    {
        nk_label(ctx, "Seed", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
    }
    nk_layout_row_push(ctx, 220);
    {
        nk_flags flags = nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, seed_buffer, sizeof(seed_buffer) - 1, nk_filter_decimal);
        if (flags & NK_EDIT_DEACTIVATED)
        {
            config.debug.seed = strtol(seed_buffer, NULL, 10);
            config_save();
            printf("edit committed\n");
        }
    }
    nk_layout_row_end(ctx);
    nk_layout_row_dynamic(ctx, 60, 1);
    nk_label_wrap(ctx, "The above seed will be used for your next deal");
}

void settings_render_oss(struct nk_context *ctx)
{
    // Nuklear Styles
    float group_border = ctx->style.window.group_border;
    struct nk_color group_border_color = ctx->style.window.group_border_color;
    ctx->style.window.group_border = 2;
    ctx->style.window.group_border_color = nk_rgb(255, 255, 255);

    nk_layout_row_dynamic(ctx, 40, 1);
    nk_label(ctx, "Open Source Software Licences", NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED);

    int row_wrap_height = settings_panel_width < 600 ? 48 : 24;

    int count;
    Licence *licences = licences_get(&count);
    for (int i = 0; i < count; i++)
    {
        Licence licence = licences[i];
        nk_layout_row_dynamic(ctx, row_wrap_height, 1);

        if (i > 0)
        {
            nk_spacer(ctx);
        }

        if (licence.source)
        {
            nk_labelf_wrap(ctx, "%s | %s", licence.name, licence.source);
        }
        else
        {
            nk_label_wrap(ctx, licence.name);
        }

        if (licence.author)
        {
            nk_label_wrap(ctx, licence.author);
        }
        nk_layout_row_begin(ctx, NK_DYNAMIC, 300, 2);
        nk_layout_row_push(ctx, 0.9);
        if (nk_group_begin(ctx, licence.name, NK_WINDOW_BORDER))
        {
            nk_layout_row_dynamic(ctx, row_wrap_height, 1);
            for (int i = 0; i < licence.line_count; i++)
            {
                nk_label_wrap(ctx, licence.lines[i]);
            }
            nk_group_end(ctx);
        }
        nk_layout_row_push(ctx, 0.1);
        nk_spacer(ctx);
        nk_layout_row_end(ctx);
    }

    // Reset Nuklear styles
    ctx->style.window.group_border = group_border;
    ctx->style.window.group_border_color = group_border_color;
}

void settings_nk_menu_button(struct nk_context *ctx, char *name, SettingsPages page, int spacer)
{
    struct nk_style_item normal_orig = ctx->style.button.normal;
    if (page == settings_page)
    {
        ctx->style.button.normal = nk_style_item_color(ctx->style.button.border_color);
    }

    nk_layout_row_dynamic(ctx, 30, 1);
    int clicked = nk_button_label(ctx, name);

    if (spacer)
    {
        nk_layout_row_dynamic(ctx, 2, 1);
        nk_spacing(ctx, 1);
    }

    if (page == settings_page)
    {
        ctx->style.button.normal = normal_orig;
    }

    if (clicked)
    {
        settings_page = page;
    }
}

void settings_render(struct nk_context *ctx)
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    int border = 50;
    if (width > 800 && height > 600)
    {
        border = 100;
    }
    int settings_width = width - (border * 2);
    int settings_height = height - (border * 2);
    int menu_size = 200;
    settings_panel_width = settings_width - menu_size - 60;
    if (nk_begin(ctx, "Settings", nk_rect(border, border, settings_width, settings_height), NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
        nk_layout_row_begin(ctx, NK_STATIC, settings_height - 80, 2);
        nk_layout_row_push(ctx, menu_size - 20);
        if (nk_group_begin(ctx, "menu", NULL))
        {
            settings_nk_menu_button(ctx, "Solitaire", PAGE_GAME, 1);
            settings_nk_menu_button(ctx, "Texture Packs", PAGE_TEXTURE_PACKS, 1);
            settings_nk_menu_button(ctx, "Debug", PAGE_DEBUG, 1);
            settings_nk_menu_button(ctx, "OSS", PAGE_OSS, 0);

            nk_layout_row_dynamic(ctx, 30, 1);
            nk_spacer(ctx);

            nk_layout_row_dynamic(ctx, 30, 1);
            if (nk_button_label(ctx, "Back"))
            {
                scene_pop();
            }

            nk_layout_row_dynamic(ctx, 2, 1);
            nk_spacing(ctx, 1);

            nk_layout_row_dynamic(ctx, 30, 1);
            if (nk_button_label(ctx, "Leaderboard"))
            {
                scene_pop();
                scene_push(&LeaderboardScene);
            }

            nk_layout_row_dynamic(ctx, 2, 1);
            nk_spacing(ctx, 1);

            nk_layout_row_dynamic(ctx, 30, 1);
            if (nk_button_label(ctx, "Return to menu"))
            {
                scene_pop_to(&MenuScene);
            }
            nk_group_end(ctx);
        }
        nk_layout_row_push(ctx, settings_panel_width + 20);
        if (nk_group_begin(ctx, "settings", NULL))
        {
            switch (settings_page)
            {
            case PAGE_GAME:
            {
                settings_render_game(ctx);
                break;
            }
            case PAGE_TEXTURE_PACKS:
            {
                settings_render_texture_packs(ctx);
                break;
            }
            case PAGE_DEBUG:
            {
                settings_render_debug(ctx);
                break;
            }
            case PAGE_OSS:
            {
                settings_render_oss(ctx);
                break;
            }
            default:
            {
                nk_layout_row_static(ctx, 30, settings_panel_width, 1);
                nk_label(ctx, "Invalid settings page", NK_TEXT_ALIGN_MIDDLE);
                break;
            }
            }
            nk_group_end(ctx);
        }
        nk_layout_row_end(ctx);
    }
    nk_end(ctx);
}

const Scene SettingsScene = {
    .start = settings_start,
    .stop = settings_stop,
    .update = settings_update,
    .render = settings_render,
    .popup = 1,
};
