#include "scenes/scene.h"

#include "util/util.h"

#include <raylib.h>
#include <raylib-nuklear.h>

#include <stdio.h>

Font header_font;
Font credit_font;

void menu_start()
{
    header_font = LoadFontEx("res/font/roboto/Roboto-Medium.ttf", 50, 0, 250);
    credit_font = LoadFontEx("res/font/roboto/Roboto-Medium.ttf", 24, 0, 250);
}

void menu_stop()
{
    UnloadFont(header_font);
    UnloadFont(credit_font);
}

void menu_render(struct nk_context *ctx)
{
    Vector2 size = MeasureTextEx(header_font, "solitaire", 50, 0.0f);
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    DrawTextEx(header_font, "solitaire", (Vector2){.x = (sw - size.x) / 2, .y = 100}, 50, 0.0f, BLACK);

    int menu_width = min(sw - 20, 300);
    int menu_height = 250;

    struct nk_rect menu_rect = nk_rect((sw - menu_width) / 2, sh - menu_height - 100,
                                       menu_width, menu_height);

    if (nk_begin(ctx, "Menu", menu_rect, 0))
    {
        nk_layout_row_dynamic(ctx, 10, 1);
        nk_spacer(ctx);
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

        nk_layout_row_dynamic(ctx, 10, 1);
        nk_spacer(ctx);
        nk_layout_row_dynamic(ctx, 40, 1);
        if (nk_button_label(ctx, "quit"))
        {
            scene_pop_all();
        }
        nk_end(ctx);
    }

    DrawTextEx(credit_font, "ohaizac 2023",
               (Vector2){.x = 10, .y = sh - 34}, 24, 0.0f, BLACK);
    size = MeasureTextEx(credit_font, "made with C and Raylib", 24, 0.0f);
    DrawTextEx(credit_font, "made with C and Raylib",
               (Vector2){.x = sw - size.x - 10, .y = sh - 34}, 24, 0.0f, BLACK);
}

const Scene MenuScene = {
    .start = menu_start,
    .stop = menu_stop,
    .render = menu_render,
};
