#include "scenes/scene.h"

#include "solitaire.h"

#include <raylib.h>
#include <stdio.h>

#define AUTO_COMPLETE_TIMEOUT 0.2f

Solitaire solitaire;
int initialized = 0;
int auto_completing;
float auto_complete_timer;

void new_game()
{
    if (initialized)
    {
        solitaire_free(&solitaire);
    }
    solitaire = solitaire_create((SolitaireConfig){
        .seed = 1672131473,
        .deal_three = 1,
    });
    auto_completing = 0;
    auto_complete_timer = 0.0f;
    cards_animate_deal(&solitaire);
    initialized = 1;
}

void start()
{
    new_game();
}

void stop()
{
    solitaire_free(&solitaire);
}

void update(float dt)
{
    // todo: disable play while auto completing - cards should not need update func
    // to render properly!!!
    cards_update(&solitaire);

    if (IsKeyPressed(KEY_R))
    {
        anim_clear_all();
        solitaire = solitaire_create((SolitaireConfig){
            .seed = 0,
            .deal_three = 1,
        });
        cards_animate_deal(&solitaire);
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

    DrawText("controls: [r] restart [z] undo [x] redo [p] pause", 10, 574, 16, GRAY);
}

const Scene GameScene = {
    .start = start,
    .stop = stop,
    .update = update,
    .render = render,
};
