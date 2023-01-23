#include "util/debug.h"

#include "gfx/cards.h"
#include "io/config.h"
#include "util/util.h"

#include <stdlib.h>

void debug_render(struct nk_context *ctx, Solitaire *solitaire)
{
    if (config.debug.render_hitboxes)
    {
        debug_hitboxes(ctx);
    }

    if (config.debug.render_animation_list)
    {
        debug_animation_list(ctx);
    }

    if (config.debug.render_leaderboard_tool)
    {
        debug_leaderboard_tool(ctx, solitaire);
    }
}

void debug_hitboxes(struct nk_context *ctx)
{
    for (int i = 0; i < MAX_CARDS; i++)
    {
        CardSprite *sprite = &cards[i];

        if (!(sprite->flags & FLAGS_HITBOX))
        {
            continue;
        }

        char buffer[256];
        snprintf(buffer, 256, "%s %s", SUITS[sprite->suit], VALUE[sprite->value]);
        DrawText(buffer, sprite->hitbox.x + 2, sprite->hitbox.y + sprite->hitbox.height - 14, 12, RED);
        DrawRectangleLinesEx(sprite->hitbox, 2, BLUE);
    }
}

void debug_animation_list(struct nk_context *ctx)
{
    struct nk_rect animation_list_bounds = nk_rect(
        config.window_size.width - 150,
        config.window_size.height - 200,
        140, 140);

    if (nk_begin(ctx, "animation list", animation_list_bounds, NK_WINDOW_BORDER))
    {
        nk_layout_row_static(ctx, 30, 120, 1);
        for (int i = 0; i < MAX_CARDS; i++)
        {
            CardSprite *sprite = &cards[i];

            if (!(sprite->flags & FLAGS_ANIMATING))
            {
                continue;
            }

            nk_labelf(ctx, NK_TEXT_LEFT, "%s %s (%.02f)", SUITS[sprite->suit],
                      VALUE[sprite->value], anim_get_duration(sprite->animPtr));
        }
    }
    nk_end(ctx);
}

void debug_leaderboard_tool(struct nk_context *ctx, Solitaire *solitaire)
{
    if (nk_begin(ctx, "Leaderboard Debug", nk_rect(10, 10, 400, 210), NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
        static char score_buffer[12] = {0}, time_buffer[12] = {0}, moves_buffer[12] = {0};

        nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
        nk_layout_row_push(ctx, 60);
        nk_label(ctx, "Score", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
        nk_layout_row_push(ctx, 140);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, score_buffer, sizeof(score_buffer) - 1,
                                       nk_filter_decimal);
        nk_layout_row_end(ctx);

        nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
        nk_layout_row_push(ctx, 60);
        nk_label(ctx, "Time", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
        nk_layout_row_push(ctx, 140);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, time_buffer, sizeof(time_buffer) - 1,
                                       nk_filter_decimal);
        nk_layout_row_end(ctx);

        nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
        nk_layout_row_push(ctx, 60);
        nk_label(ctx, "Moves", NK_TEXT_ALIGN_LEFT | NK_TEXT_ALIGN_MIDDLE);
        nk_layout_row_push(ctx, 140);
        nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, moves_buffer, sizeof(moves_buffer) - 1,
                                       nk_filter_decimal);
        nk_layout_row_end(ctx);

        nk_layout_row_dynamic(ctx, 10, 1);
        nk_spacer(ctx);

        nk_layout_row_dynamic(ctx, 30, 1);
        if (nk_button_label(ctx, "Submit"))
        {
            leaderboard_submit(solitaire->config.seed, strtol(score_buffer, NULL, 10),
                               strtol(time_buffer, NULL, 10), strtol(moves_buffer, NULL, 10));
        }

        nk_end(ctx);
    }
}