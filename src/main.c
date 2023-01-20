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
    // init utilities
    config_load();
    leaderboard_load();
    licences_load();

    // init raylib
    SetExitKey(KEY_NULL);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    if (config.fullscreen)
    {
        SetConfigFlags(FLAG_FULLSCREEN_MODE);
    }

    InitWindow(config.window_size.width, config.window_size.height, "solitaire");

    Font font = LoadFontEx("res/font/roboto/Roboto-Medium.ttf", 20, 0, 250);
    struct nk_context *ctx = InitNuklearEx(font, 20);
    ctx->style.checkbox.cursor_hover = nk_style_item_color(nk_rgb(255, 255, 255));
    ctx->style.checkbox.cursor_normal = nk_style_item_color(nk_rgb(255, 255, 255));

    // init physfs
    PHYSFS_init(argv[0]);

    // init game components
    pacman_reload_packs();
    audio_init();
    cards_init();
    scene_push(&GameScene);
    layout_resize();

    // main loop
    while (!WindowShouldClose())
    {
        /* UPDATE */
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

        /* RENDER */
        BeginDrawing();

        ClearBackground(RAYWHITE);
        scene_render(ctx);
        DrawNuklear(ctx);

        EndDrawing();
    }

    // cleanup game components
    anim_release();

    scene_pop_all();
    cards_free();
    audio_free();
    pacman_free_packs();

    // cleanup physfs
    PHYSFS_deinit();

    // cleanup raylib
    UnloadNuklear(ctx);
    UnloadFont(font);
    CloseWindow();

    // cleanup utilities
    licences_free();
    config_save();
    config_free();
    return 0;
}