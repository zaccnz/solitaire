#include "scenes/scene.h"

#include <raylib.h>
#include <raylib-nuklear.h>
#include <stdio.h>

void settings_start()
{
    printf("started settings scene\n");
}

void settings_stop()
{
    printf("stopped settings scene\n");
}

void settings_update(float dt)
{
    if (IsKeyPressed(KEY_P))
    {
        scene_pop();
    }
}

void settings_render(struct nk_context *ctx)
{
    DrawText("the game is paused!", 190, 200, 20, LIGHTGRAY);
    DrawText("press [p] to unpause!", 190, 220, 20, LIGHTGRAY);

    if (nk_begin(ctx, "Nuklear", nk_rect(100, 100, 400, 100), NK_WINDOW_BORDER))
    {
        nk_layout_row_static(ctx, 30, 200, 1);
        nk_label(ctx, "Hello world", NK_TEXT_LEFT);

        nk_layout_row_static(ctx, 30, 80, 1);
        if (nk_button_label(ctx, "resume"))
        {
            scene_pop();
        }
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
