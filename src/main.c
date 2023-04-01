#include "gfx/animator.h"
#include "gfx/background.h"
#include "gfx/cards.h"
#include "gfx/layout.h"
#include "io/config.h"
#include "io/licences.h"
#include "io/leaderboard.h"
#include "io/pacman.h"
#include "scenes/scene.h"
#include "sfx/audio.h"
#include "util/emscripten.h"
#include "util/util.h"

#include <physfs.h>
#include <raylib.h>

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include <raylib-nuklear.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

struct nk_context *ctx = NULL;

void loop()
{
    /* UPDATE */
    UpdateNuklear(ctx);

    audio_update();
    anim_update();

    if (IsWindowResized())
    {
        if (!IsWindowFullscreen())
        {
            printf("updating window width, height (%d,%d)\n", config.window_size.width, config.window_size.height);
            config.window_size.width = GetScreenWidth();
            config.window_size.height = GetScreenHeight();
            config_save();
        }
        layout_resize();
        anim_resize();
    }

    scene_update(GetFrameTime());

    /* RENDER */
    BeginDrawing();

    Assets *bg_assets = pacman_get_current_assets(ASSET_BACKGROUNDS);
    if (bg_assets->background.type == BACKGROUND_COLOUR)
    {
        ClearBackground(bg_assets->background.colour);
    }
    else
    {
        ClearBackground(RAYWHITE);
        background_render(bg_assets);
    }

    scene_render(ctx);
    anim_render();
    DrawNuklear(ctx);

    EndDrawing();
}

int main(int argc, char **argv)
{
    // prepare IDBFS
    emscripten_idbfs_prepare();

    // init utilities
    config_load();
    leaderboard_load();
    licences_load();

    // init raylib
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    int width = config.window_size.width;
    int height = config.window_size.height;
    if (config.fullscreen)
    {
        SetConfigFlags(FLAG_FULLSCREEN_MODE);
        width = 0, height = 0;
    }

    InitWindow(width, height, "solitaire");
    SetExitKey(KEY_NULL);

    Font font = LoadFontEx("res/font/roboto/Roboto-Medium.ttf", 20, 0, 250);
    ctx = InitNuklearEx(font, 20);
    ctx->style.checkbox.cursor_hover = nk_style_item_color(nk_rgb(255, 255, 255));
    ctx->style.checkbox.cursor_normal = nk_style_item_color(nk_rgb(255, 255, 255));

    // init physfs
    PHYSFS_init(argv[0]);

    // init game components
    pacman_reload_packs();
    audio_init();
    cards_init();
    scene_push(&MenuScene);
    scene_update(0.0);
    layout_resize();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(loop, 0, 1);
#else
    while (!WindowShouldClose() && scene_count > 0)
    {
        loop();
    }
#endif

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

    // sync IDBFS
    emscripten_idbfs_sync();

    return 0;
}