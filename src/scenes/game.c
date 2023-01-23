#include "scenes/scene.h"

#include "gfx/animations.h"
#include "gfx/animator.h"
#include "io/config.h"
#include "io/leaderboard.h"
#include "util/debug.h"
#include "util/util.h"
#include "solitaire.h"

#include <raylib.h>
#include <stdio.h>

#define AUTO_COMPLETE_TIMEOUT 0.2f

Solitaire solitaire;

int auto_completing = 0.0f;
float auto_complete_timer = 0.0f;
int did_complete = 0;
float ten_second_timer = 0.0f;

int show_autocomplete_window = 0;
int hide_autocomplete_window = 0;

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
    game_new_deal(0);
}

void stop()
{
    solitaire_free(&solitaire);
}

void update(float dt, int background)
{
    cards_update(&solitaire, background || auto_completing);

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
        solitaire.score.points += solitaire_score_move(&solitaire, SCORE_FINISH_GAME, NULL, NULL);
        leaderboard_submit(solitaire.config.seed, solitaire.score.points,
                           (int)solitaire.score.elapsed, solitaire.score.user_moves);
        did_complete = 1;
    }
}

void render(struct nk_context *ctx)
{
    cards_render(&solitaire, ctx);
    debug_render(ctx, &solitaire);

    // TODO: render text with font, and use layout system to get position
    char text[2048];

    if (solitaire.config.timed)
    {
        float elapsed = solitaire.score.elapsed;

        snprintf(text, 2048, "Time: %d:%02d", (int)(elapsed / 60), (int)(elapsed) % 60);
        DrawText(text, 10, 510, 16, GRAY);
    }

    snprintf(text, 2048, "Moves: %d", solitaire.score.user_moves);
    DrawText(text, 10, 526, 16, GRAY);

    snprintf(text, 2048, "Score: %d", solitaire.score.points);
    DrawText(text, 10, 542, 16, GRAY);

    int sw = GetScreenWidth(), sh = GetScreenHeight();

    int action_width = min(sw - 20, 300);
    int action_height = min(sh - 40, 250);

    struct nk_rect action_bounds = nk_rect((sw - action_width) / 2,
                                           (sh - action_height) / 2,
                                           action_width,
                                           action_height);

    if (solitaire_is_complete(&solitaire) && nk_begin(ctx, "Game Complete", action_bounds, NULL))
    {
        nk_layout_row_dynamic(ctx, 40, 1);
        nk_label(ctx, "Congratulations", NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED);

        nk_layout_row_dynamic(ctx, 24, 1);
        nk_label(ctx, "Game Completed", NK_TEXT_ALIGN_LEFT);
        if (solitaire.config.timed)
        {
            float elapsed = solitaire.score.elapsed;
            nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "Time: %d:%02d",
                      (int)(elapsed / 60), (int)(elapsed) % 60);
        }

        nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "Score: %d", solitaire.score.points);
        nk_labelf(ctx, NK_TEXT_ALIGN_LEFT, "Moves: %d", solitaire.score.user_moves);

        nk_layout_row_dynamic(ctx, 30, 1);
        nk_spacer(ctx);
        if (nk_button_label(ctx, "New Deal"))
        {
            game_new_deal(0);
        }

        nk_end(ctx);
    }

    if (solitaire_can_auto_complete(&solitaire) && !show_autocomplete_window && !hide_autocomplete_window)
    {
        show_autocomplete_window = 1;
    }

    if (show_autocomplete_window && nk_begin(ctx, "Autocomplete", action_bounds, NULL))
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

    int ctrl_height = 80;
    int ctrl_pad = 10;
    int ctrl_width = min(sw - (ctrl_pad * 2), 500);

    struct nk_rect control_bounds = nk_rect((sw - ctrl_width) / 2,
                                            sh - ctrl_height - ctrl_pad,
                                            ctrl_width,
                                            ctrl_height);

    if (nk_begin(ctx, "Game Controls", control_bounds, NULL))
    {
        nk_layout_row_dynamic(ctx, ctrl_height - 20, 4);
        nk_end(ctx);
    }
}

const Scene GameScene = {
    .start = start,
    .stop = stop,
    .update = update,
    .render = render,
};
