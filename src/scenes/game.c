#include "scenes/scene.h"

#include "gfx/animations.h"
#include "gfx/animator.h"
#include "gfx/cards.h"
#include "gfx/layout.h"
#include "io/config.h"
#include "io/leaderboard.h"
#include "sfx/audio.h"
#include "util/debug.h"
#include "util/util.h"
#include "solitaire.h"

#include <raylib.h>
#include <raylib-nuklear.h>
#include <stdio.h>

#define AUTO_COMPLETE_TIMEOUT 0.2f

Solitaire solitaire;

Font score_font;
Font score_header_font;

typedef enum UIICONS
{
    ICON_UNDO = 0,
    ICON_REDO,
    ICON_NEW_GAME,
    ICON_LEADERBOARD,
    ICON_SETTINGS,
    ICON_MAX,
} UI_Icons;

const char *ICON_DIRECTORY = "res/tex/Icon";
const char *ICON_FILENAMES[ICON_MAX] = {
    "undo-circular-arrow.png",
    "redo-arrow-symbol.png",
    "plus.png",
    "podium.png",
    "gear.png",
};
struct nk_image nk_icons[ICON_MAX];

int auto_completing = 0.0f;
float auto_complete_timer = 0.0f;
int did_complete = 0;
float ten_second_timer = 0.0f;

int show_autocomplete_window = 0;
int hide_autocomplete_window = 0;

int skip_hold = 0;

void game_new_deal(int seed)
{
    if (solitaire.config.seed != 0)
    {
        solitaire_free(&solitaire);
    }

    solitaire = solitaire_create((SolitaireConfig){
        .seed = seed == 0 ? config.debug.seed : seed,
        .deal_three = config.solitaire.dealthree,
        .timed = config.solitaire.timed,
    });

    config.debug.seed = 0;
    config_save();

    auto_completing = 0;
    auto_complete_timer = 0.0f;
    did_complete = 0;
    ten_second_timer = 0.0f;

    show_autocomplete_window = 0;
    hide_autocomplete_window = 0;

    anim_clear_all();
    animation_deal(&solitaire);
}

void start()
{
    char path[2048];
    for (int i = 0; i < ICON_MAX; i++)
    {
        snprintf(path, 2048, "%s/%s", ICON_DIRECTORY, ICON_FILENAMES[i]);
        nk_icons[i] = LoadNuklearImage(path);
    }

    score_font = LoadFontEx("res/font/roboto/Roboto-Medium.ttf", 30, 0, 250);
    score_header_font = LoadFontEx("res/font/roboto/Roboto-Medium.ttf", 18, 0, 250);

    game_new_deal(0);
}

void stop()
{
    UnloadFont(score_font);
    UnloadFont(score_header_font);

    for (int i = 0; i < ICON_MAX; i++)
    {
        UnloadNuklearImage(nk_icons[i]);
    }
    solitaire_free(&solitaire);
}

void play()
{
    skip_hold = 1;
}

void update(float dt, int background)
{
    cards_update(&solitaire, background || auto_completing, skip_hold);

    if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
    {
        skip_hold = 0;
    }

    if (background)
    {
        return;
    }

    if (solitaire.score.user_moves > 0 && !solitaire_is_complete(&solitaire))
    {
        solitaire.score.elapsed += dt;
        ten_second_timer += dt;
        if (ten_second_timer > 10.0)
        {
            solitaire.score.points += solitaire_score_move(&solitaire, SCORE_TEN_SECONDS, NULL, NULL);
            ten_second_timer -= 10.0;
        }
    }

    if (IsKeyPressed(KEY_R))
    {
        game_new_deal(0);
    }

    if (IsKeyPressed(KEY_P))
    {
        scene_push(&SettingsScene);
    }

    if (IsKeyPressed(KEY_Z))
    {
        solitaire_undo(&solitaire);
    }
    if (IsKeyPressed(KEY_X))
    {
        solitaire_redo(&solitaire);
    }

    int can_auto_complete = solitaire_can_auto_complete(&solitaire);

    if (can_auto_complete && !show_autocomplete_window && !hide_autocomplete_window)
    {
        show_autocomplete_window = 1;
    }

    if (can_auto_complete && IsKeyPressed(KEY_C))
    {
        auto_completing = 1;
        auto_complete_timer = AUTO_COMPLETE_TIMEOUT;
    }

    if (auto_completing)
    {
        if (auto_complete_timer <= 0.0f)
        {
            solitaire_auto_complete_move(&solitaire);

            if (solitaire_is_complete(&solitaire))
            {
                auto_completing = false;
            }
            auto_complete_timer = AUTO_COMPLETE_TIMEOUT;
        }
        auto_complete_timer -= GetFrameTime();
    }

    if (solitaire_is_complete(&solitaire) && did_complete == 0)
    {
        audio_play_sfx(SFX_GAME_WIN);
        animation_game_end();
        solitaire.score.points += solitaire_score_move(&solitaire, SCORE_FINISH_GAME, NULL, NULL);
        leaderboard_submit(solitaire.config.seed, solitaire.score.points,
                           (int)solitaire.score.elapsed, solitaire.score.user_moves);
        did_complete = 1;
    }
}

void game_render_text_centered(char *text, int x, int y, Font font, float size)
{
    Vector2 dim = MeasureTextEx(font, text, size, 0);
    dim.x = x - (dim.x / 2);
    dim.y = y;
    DrawTextEx(font, text, dim, size, 0, BLACK);
}

void game_render_score(int sw, int sh)
{
    CalcOut backdrop;
    layout_calculate(LAYOUT_SCORE, NULL, &backdrop);
    DrawRectangleRounded(layout_calcout_to_rayrect(backdrop),
                         0.5, 20, ColorAlpha(WHITE, 0.8));

    int columns = backdrop.width / 4;

    // TODO: render text with font, and use layout system to get position
    char text[2048];

    if (solitaire.config.timed)
    {
        float elapsed = solitaire.score.elapsed;

        game_render_text_centered("Time", backdrop.x + columns * 2, 12, score_header_font, 18);
        snprintf(text, 2048, "%d:%02d", (int)(elapsed / 60), (int)(elapsed) % 60);
        game_render_text_centered(text, backdrop.x + columns * 2, 28, score_font, 30);
    }

    game_render_text_centered("Moves", backdrop.x + columns, 12, score_header_font, 18);
    snprintf(text, 2048, "%d", solitaire.score.user_moves);
    game_render_text_centered(text, backdrop.x + columns, 28, score_font, 30);

    game_render_text_centered("Score", backdrop.x + columns * 3, 12, score_header_font, 18);
    snprintf(text, 2048, "%d", solitaire.score.points);
    game_render_text_centered(text, backdrop.x + columns * 3, 28, score_font, 30);
}

void game_nk_gameover(struct nk_context *ctx, struct nk_rect action_bounds)
{
    if (nk_begin(ctx, "Game Complete", action_bounds, 0))
    {
        nk_layout_row_dynamic(ctx, 40, 1);
        nk_label(ctx, "Congratulations", NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED);

        nk_layout_row_dynamic(ctx, 24, 2);
        nk_spacing(ctx, 2);
        if (solitaire.config.timed)
        {
            float elapsed = solitaire.score.elapsed;
            nk_label(ctx, "Time:", NK_TEXT_ALIGN_RIGHT);
            nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, " %d:%02d",
                      (int)(elapsed / 60), (int)(elapsed) % 60);
        }

        nk_label(ctx, "Score:", NK_TEXT_ALIGN_RIGHT);
        nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, " %d", solitaire.score.points);
        nk_label(ctx, "Moves:", NK_TEXT_ALIGN_RIGHT);
        nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, " %d", solitaire.score.user_moves);

        nk_layout_row_dynamic(ctx, 30, 1);
        nk_spacer(ctx);
        if (nk_button_label(ctx, "New Deal"))
        {
            game_new_deal(0);
        }

        nk_end(ctx);
    }
}

void game_nk_autocomplete(struct nk_context *ctx, struct nk_rect action_bounds)
{
    if (nk_begin(ctx, "Autocomplete", action_bounds, 0))
    {
        nk_layout_row_dynamic(ctx, 10, 1);
        nk_spacer(ctx);
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "This game can be completed", NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED);
        nk_label(ctx, "automatically", NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED);

        nk_layout_row_dynamic(ctx, 80, 1);
        nk_spacer(ctx);

        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_button_label(ctx, "Auto-complete"))
        {
            show_autocomplete_window = 0;
            hide_autocomplete_window = 1;
            auto_completing = 1;
            auto_complete_timer = AUTO_COMPLETE_TIMEOUT;
        }

        if (nk_button_label(ctx, "Back"))
        {
            show_autocomplete_window = 0;
            hide_autocomplete_window = 1;
        }

        nk_end(ctx);
    }
}

void game_nk_controls(struct nk_context *ctx, int sw, int sh)
{
    CalcOut control_bounds;
    layout_calculate(LAYOUT_CONTROLS, NULL, &control_bounds);

    int icon_width = control_bounds.height - 10;

    int divider = max((control_bounds.width - (icon_width + 6) * 5) / 2, 0);

    float column_widths[7] = {
        icon_width,
        icon_width,
        divider,
        icon_width,
        divider,
        icon_width,
        icon_width,
    };

    nk_style_push_color(ctx, &ctx->style.window.background, nk_rgba(0, 0, 0, 255));
    nk_style_push_style_item(ctx, &ctx->style.window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));

    nk_style_push_style_item(ctx, &ctx->style.button.normal, nk_style_item_color(nk_rgba(255, 255, 255, (0.8 * 255.0))));
    nk_style_push_style_item(ctx, &ctx->style.button.hover, nk_style_item_color(nk_rgba(240, 240, 240, (0.8 * 255.0))));
    nk_style_push_style_item(ctx, &ctx->style.button.active, nk_style_item_color(nk_rgba(200, 200, 200, (0.8 * 255.0))));
    nk_style_push_color(ctx, &ctx->style.button.border_color, nk_rgba(0, 0, 0, 0));
    if (nk_begin(ctx, "Game Controls", layout_calcout_to_nkrect(control_bounds), NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row(ctx, NK_STATIC, control_bounds.height - 10, 7, column_widths);
        if (nk_button_image(ctx, nk_icons[ICON_UNDO]))
        {
            solitaire_undo(&solitaire);
        }

        if (nk_button_image(ctx, nk_icons[ICON_REDO]))
        {
            solitaire_redo(&solitaire);
        }

        nk_spacer(ctx);

        if (nk_button_image(ctx, nk_icons[ICON_NEW_GAME]))
        {
            game_new_deal(0);
        }

        nk_spacer(ctx);

        if (nk_button_image(ctx, nk_icons[ICON_LEADERBOARD]))
        {
            scene_push(&LeaderboardScene);
        }

        if (nk_button_image(ctx, nk_icons[ICON_SETTINGS]))
        {
            scene_push(&SettingsScene);
        }
        nk_end(ctx);
    }
    nk_style_pop_style_item(ctx);
    nk_style_pop_style_item(ctx);
    nk_style_pop_style_item(ctx);
    nk_style_pop_color(ctx);
    nk_style_pop_style_item(ctx);
    nk_style_pop_color(ctx);
}

void render(struct nk_context *ctx)
{
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    CalcOut action_bounds_calc;
    layout_calculate(LAYOUT_ACTION, NULL, &action_bounds_calc);
    struct nk_rect action_bounds = layout_calcout_to_nkrect(action_bounds_calc);

    game_render_score(sw, sh);

    cards_render(&solitaire, ctx);
    debug_render(ctx, &solitaire);

    game_nk_controls(ctx, sw, sh);

    if (solitaire_is_complete(&solitaire))
    {
        game_nk_gameover(ctx, action_bounds);
    }

    if (show_autocomplete_window)
    {
        game_nk_autocomplete(ctx, action_bounds);
    }
}

const Scene GameScene = {
    .start = start,
    .stop = stop,
    .play = play,
    .update = update,
    .render = render,
};
