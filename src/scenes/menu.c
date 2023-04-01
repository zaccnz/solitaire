#include "scenes/scene.h"

#include "gfx/animations.h"
#include "gfx/animator.h"
#include "util/util.h"

#include <raylib.h>
#include <raylib-nuklear.h>

#include <stdio.h>

Font header_font;
Font credit_font;

void menu_play()
{
    animation_main_menu();
}

void menu_pause()
{
    anim_clear_all();
}

void menu_start()
{
    header_font = LoadFontEx("res/font/roboto/Roboto-Medium.ttf", 50, 0, 250);
    credit_font = LoadFontEx("res/font/roboto/Roboto-Medium.ttf", 24, 0, 250);
    menu_play();
}

void menu_stop()
{
    menu_pause();
    UnloadFont(header_font);
    UnloadFont(credit_font);
}

void menu_render(struct nk_context *ctx)
{
    Vector2 size = MeasureTextEx(header_font, "solitaire", 50, 0.0f);
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    DrawTextEx(header_font, "solitaire", (Vector2){.x = (sw - size.x) / 2, .y = 50}, 50, 0.0f, BLACK);

    int menu_width = min(sw - 20, 300);
    int button_count = 4;
#if defined(PLATFORM_WEB)
    button_count = 3;
#endif
    int border_bottom = (sh < 600) ? 20 : 100;
    int menu_height = button_count * 60;

    struct nk_rect menu_rect = nk_rect((sw - menu_width) / 2,
                                       sh - menu_height - border_bottom,
                                       menu_width, menu_height);

    nk_style_push_color(ctx, &ctx->style.window.background, nk_rgba(0, 0, 0, 255));
    nk_style_push_style_item(ctx, &ctx->style.window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));

    if (nk_begin(ctx, "Menu", menu_rect, 0))
    {
        nk_layout_row_dynamic(ctx, 40, 1);
        if (nk_button_label(ctx, "play"))
        {
            scene_push(&GameScene);
        }

        nk_layout_row_dynamic(ctx, 10, 1);
        nk_spacer(ctx);
        nk_layout_row_dynamic(ctx, 40, 1);
        if (nk_button_label(ctx, "settings"))
        {
            scene_push(&SettingsScene);
        }

        nk_layout_row_dynamic(ctx, 10, 1);
        nk_spacer(ctx);
        nk_layout_row_dynamic(ctx, 40, 1);
        if (nk_button_label(ctx, "leaderboard"))
        {
            scene_push(&LeaderboardScene);
        }
#ifndef PLATFORM_WEB
        nk_layout_row_dynamic(ctx, 10, 1);
        nk_spacer(ctx);
        nk_layout_row_dynamic(ctx, 40, 1);
        if (nk_button_label(ctx, "quit"))
        {
            scene_pop_all();
        }
#endif
        nk_end(ctx);
    }

    nk_style_pop_style_item(ctx);
    nk_style_pop_color(ctx);

    DrawTextEx(credit_font, "ohaizac 2023",
               (Vector2){.x = 10, .y = sh - 34}, 24, 0.0f, BLACK);
    size = MeasureTextEx(credit_font, "made with C and Raylib", 24, 0.0f);
    DrawTextEx(credit_font, "made with C and Raylib",
               (Vector2){.x = sw - size.x - 10, .y = sh - 34}, 24, 0.0f, BLACK);
}

const Scene MenuScene = {
    .start = menu_start,
    .stop = menu_stop,
    .play = menu_play,
    .pause = menu_pause,
    .render = menu_render,
};
