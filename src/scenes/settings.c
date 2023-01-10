#include "scenes/scene.h"

#include "io/pacman.h"

#include "util.h"

#include <raylib.h>
#include <raylib-nuklear.h>
#include <stdio.h>
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

void data_add_pointer(struct DropdownData *data, PackPointer *ptr)
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
    data->pointers[data->length++] = ptr;
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

    for (int i = 0; i < min(count, 256); i++)
    {
        PackPointer *ptr = &pointers[i];
        if (ptr->type & TEXTURE_BACKGROUNDS)
        {
            data_add_pointer(&background_data, ptr);
        }
        if (ptr->type & TEXTURE_BACKS)
        {
            data_add_pointer(&backs_data, ptr);
        }
        if (ptr->type & TEXTURE_CARDS)
        {
            data_add_pointer(&cards_data, ptr);
        }
    }
}

void settings_start()
{
    printf("started settings scene\n");
    settings_was_opened = 1;
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
    printf("settings background %d\n", background);
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

void settings_render(struct nk_context *ctx)
{
    if (nk_begin(ctx, "Nuklear", nk_rect(100, 100, 400, 300), NK_WINDOW_BORDER))
    {
        nk_layout_row_static(ctx, 30, 120, 1);
        if (nk_button_label(ctx, "resume"))
        {
            scene_pop();
        }
        if (nk_button_label(ctx, "reload textures"))
        {
            // pacman_reload_packs(); // crashes the app >:(
            settings_refresh_dropdown();
        }

        /* default combobox */
        nk_layout_row_static(ctx, 30, 200, 1);
        settings_nk_draw_dropdown(ctx, "Background", TEXTURE_BACKGROUNDS, &background_data);
        settings_nk_draw_dropdown(ctx, "Card Back", TEXTURE_BACKS, &backs_data);
        settings_nk_draw_dropdown(ctx, "Cards", TEXTURE_CARDS, &cards_data);
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
