#include "gfx/animator.h"
#include "gfx/cards.h"
#include "io/config.h"
#include "io/leaderboard.h"
#include "io/pacman.h"
#include "scenes/scene.h"
#include "sfx/audio.h"

#include <physfs.h>
#include <raylib.h>

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include <raylib-nuklear.h>

int main(int argc, char **argv)
{
    config_load();
    leaderboard_load();

    SetExitKey(KEY_NULL);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    if (config.fullscreen)
    {
        SetConfigFlags(FLAG_FULLSCREEN_MODE);
    }

    InitWindow(config.window_size.width, config.window_size.height, "solitaire");

    struct nk_context *ctx = InitNuklear(10);

    PHYSFS_init(argv[0]);

    pacman_reload_packs();

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
            config.window_size.width = GetScreenWidth();
            config.window_size.height = GetScreenHeight();
            config_save();
            layout_resize();
        }

        scene_update(GetFrameTime());

        BeginDrawing();
        ClearBackground(RAYWHITE);

        scene_render(ctx);

        DrawNuklear(ctx);

        EndDrawing();
    }

    scene_pop_all();

    anim_release();
    cards_free();
    audio_free();
    pacman_free_packs();

    config_save();
    config_free();

    PHYSFS_deinit();

    UnloadNuklear(ctx);

    CloseWindow();
    return 0;
}