#include "gfx/animated.h"
#include "gfx/cards.h"
#include "scene.h"
#include "solitaire.h"
#include "scenes/menu.h"
#include "util.h"

#include <raylib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <rlgl.h>

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include <raylib-nuklear.h>

#define AUTO_COMPLETE_TIMEOUT 0.2f

int main(void)
{
    // load settings.toml
    // scene_push(&MenuScene);

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "solitaire");

    struct nk_context *ctx = InitNuklear(10);

    Solitaire solitaire = solitaire_create((SolitaireConfig){
        .seed = 1672131473,
        .deal_three = 1,
    });

    int auto_completing = 0;
    float auto_complete_timer = 0;

    cards_init();
    cards_animate_deal(&solitaire);

    while (!WindowShouldClose())
    {
        int can_auto_complete = solitaire_can_auto_complete(&solitaire);

        anim_update();
        // todo: disable while auto completing - cards should not need update func
        // to render properly!!!
        cards_update(&solitaire);

        // scene_update(0.0);

        if (IsKeyPressed(KEY_R))
        {
            solitaire = solitaire_create((SolitaireConfig){
                .seed = 0,
                .deal_three = 1,
            });
            cards_animate_deal(&solitaire);
        }

        if (IsKeyPressed(KEY_Z))
        {
            solitaire_undo(&solitaire);
        }
        if (IsKeyPressed(KEY_X))
        {
            solitaire_redo(&solitaire);
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

        UpdateNuklear(ctx);

        /*
        if (nk_begin(ctx, "Nuklear", nk_rect(100, 500, 50, 50), NULL))
        {
            if (nk_button_label(ctx, "Button"))
            {
                // Button was clicked!
            }
        }
        nk_end(ctx);*/
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // scene_render();

        cards_render(&solitaire);

        if (solitaire_is_complete(&solitaire))
        {
            DrawText("game complete! well done", 360, 370, 30, GRAY);
        }

        if (can_auto_complete)
        {
            DrawText("press [c] to autocomplete", 10, 558, 16, GRAY);
        }

        DrawText("controls: [r] restart [z] undo [x] redo", 10, 574, 16, GRAY);

        DrawNuklear(ctx);

        EndDrawing();
    }

    anim_release();
    cards_free();

    // scene_pop();
    UnloadNuklear(ctx);

    CloseWindow();

    solitaire_free(&solitaire);
    return 0;
}