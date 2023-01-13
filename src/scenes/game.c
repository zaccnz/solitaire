#include "scenes/scene.h"

#include "gfx/animations.h"
#include "io/config.h"
#include "io/leaderboard.h"
#include "solitaire.h"

#include <raylib.h>
#include <stdio.h>

#define AUTO_COMPLETE_TIMEOUT 0.2f

Solitaire solitaire;

int auto_completing;
float auto_complete_timer;
int did_complete = 0;
float ten_second_timer;

void new_game(int first)
{
    if (solitaire.config.seed != 0)
    {
        solitaire_free(&solitaire);
    }

    solitaire = solitaire_create((SolitaireConfig){
        .seed = config.debug.seed,
        .deal_three = config.solitaire.dealthree,
        .timed = config.solitaire.timed,
    });

    config.debug.seed = 0;
    config_save();

    auto_completing = 0;
    auto_complete_timer = 0.0f;
    did_complete = 0;
    ten_second_timer = 0.0f;

    animation_deal(&solitaire);
}

void start()
{
    new_game(1);
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
        new_game(0);
    }

    if (IsKeyPressed(KEY_P))
    {
        printf("pushing settings scene \n");
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
    int can_auto_complete = solitaire_can_auto_complete(&solitaire);

    cards_render(&solitaire);

    if (solitaire_is_complete(&solitaire))
    {
        DrawText("game complete! well done", 360, 370, 30, GRAY);
    }

    if (can_auto_complete)
    {
        DrawText("press [c] to autocomplete", 10, 558, 16, GRAY);
    }

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

    DrawText("controls: [r] restart [z] undo [x] redo [p] pause", 10, 574, 16, GRAY);
}

const Scene GameScene = {
    .start = start,
    .stop = stop,
    .update = update,
    .render = render,
};
