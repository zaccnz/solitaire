#include "gfx/animated.h"
#include "gfx/cards.h"
#include "io/pacman.h"
#include "scenes/scene.h"
#include "sfx/audio.h"

#include <physfs.h>
#include <raylib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <rlgl.h>

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include <raylib-nuklear.h>

int main(int argc, char **argv)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "solitaire");

    struct nk_context *ctx = InitNuklear(10);

    PHYSFS_init(argv[0]);

    // load settings.toml

    pacman_reload_packs();

    int pack = 0;
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

    scene_pop();

    anim_release();
    cards_free();
    audio_free();
    pacman_free_packs();

    PHYSFS_deinit();

    UnloadNuklear(ctx);

    CloseWindow();
    return 0;
}