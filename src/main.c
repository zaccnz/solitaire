#include "gfx/animated.h"
#include "gfx/cards.h"
#include "sfx/audio.h"
#include "scenes/scene.h"
#include "solitaire.h"
#include "util.h"

#include <raylib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <rlgl.h>

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include <raylib-nuklear.h>

int main(void)
{
    // load settings.toml

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "solitaire");

    struct nk_context *ctx = InitNuklear(10);

    audio_init();
    cards_init();

    scene_push(&GameScene);

    while (!WindowShouldClose())
    {
        UpdateNuklear(ctx);

        audio_update();
        anim_update();

        if (IsWindowResized())
        {
            layout_resize();
        }

        scene_update(GetFrameTime());

        BeginDrawing();
        ClearBackground(RAYWHITE);

        scene_render(ctx);

        DrawNuklear(ctx);

        EndDrawing();
    }

    anim_release();
    cards_free();
    audio_free();

    scene_pop();
    UnloadNuklear(ctx);

    CloseWindow();
    return 0;
}