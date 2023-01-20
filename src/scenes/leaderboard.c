#include "scenes/scene.h"

#include "io/leaderboard.h"

#include <raylib.h>
#include <raylib-nuklear.h>

int leaderboard_width;

void leaderboard_start()
{
    printf("started leaderboard scene\n");
}

void leaderboard_stop()
{
    printf("stopped leaderboard scene\n");
}

void leaderboard_nk_best_scores(struct nk_context *ctx)
{
    nk_layout_row_dynamic(ctx, 40, 3);

    if (leaderboard.lowest_moves >= 0)
    {
        nk_labelf(ctx, NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED, "Lowest Moves: %d", leaderboard.lowest_moves);
    }
    else
    {
        nk_label(ctx, "No lowest moves", NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED);
    }

    if (leaderboard.fastest_time >= 0)
    {
        int minutes = leaderboard.fastest_time / 60;
        float seconds = leaderboard.fastest_time % 60;

        nk_labelf(ctx, NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED, "Fastest Time: %d:%02.03f", minutes, seconds);
    }
    else
    {
        nk_label(ctx, "No fastest time", NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED);
    }

    if (leaderboard.high_score >= 0)
    {
        nk_labelf(ctx, NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED, "Highscore: %d", leaderboard.high_score);
    }
    else
    {
        nk_label(ctx, "No highscore", NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED);
    }
}

void leaderboard_nk_score(struct nk_context *ctx, int index, LeaderboardEntry entry)
{
    nk_layout_row_begin(ctx, NK_STATIC, 100, 3);
    nk_layout_row_push(ctx, 40);
    nk_labelf(ctx, NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED, "%d", index + 1);
    nk_layout_row_push(ctx, leaderboard_width - 240);
    if (nk_group_begin(ctx, "Leaderboard Entry", NULL))
    {
        int entry_minutes = entry.time / 60;
        float entry_seconds = entry.time % 60;
        nk_layout_row_dynamic(ctx, 24, 1);
        nk_labelf(ctx, NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_LEFT, "Moves: %d, Time: %d:%02.03f, Score: %d.",
                  entry.moves, entry_minutes, entry_seconds, entry.score);
        nk_labelf(ctx, NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_LEFT, "Achieved on: (timestamp here)");
        nk_labelf(ctx, NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_LEFT, "Seed: %d", entry.seed);
        nk_group_end(ctx);
    }
    nk_layout_row_push(ctx, 140);
    if (nk_group_begin(ctx, "Leaderboard Button", NULL))
    {
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_spacer(ctx);

        nk_layout_row_dynamic(ctx, 24, 1);
        if (nk_button_label(ctx, "Play deal"))
        {
            if (scene_is_on_stack(&GameScene))
            {
                scene_pop_to(&GameScene);
            }
            else
            {
                scene_pop();
                scene_push(&GameScene);
            }
            game_new_deal(entry.seed);
        }

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_spacer(ctx);
        nk_group_end(ctx);
    }
    nk_layout_row_end(ctx);
}

void leaderboard_render(struct nk_context *ctx)
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    int border = 50;
    if (width > 800 && height > 600)
    {
        border = 100;
    }
    leaderboard_width = width - (border * 2);
    int leaderboard_height = height - (border * 2);
    if (nk_begin(ctx, "Leaderboard", nk_rect(border, border, leaderboard_width, leaderboard_height), NK_WINDOW_BORDER | NK_WINDOW_TITLE))
    {
        leaderboard_nk_best_scores(ctx);

        nk_layout_row_dynamic(ctx, 40, 1);
        nk_spacing(ctx, 1);
        if (leaderboard.entry_count > 0)
        {
            nk_labelf(ctx, NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED, "Top %d deal%s by score",
                      leaderboard.entry_count, leaderboard.entry_count == 1 ? "" : "s");
            nk_layout_row_dynamic(ctx, leaderboard_height - 300, 1);
            if (nk_group_begin(ctx, "Leaderboard Top Scores", NULL))
            {
                for (int i = 0; i < leaderboard.entry_count; i++)
                {
                    nk_layout_row_dynamic(ctx, 10, 1);
                    if (i > 0)
                    {
                        nk_spacer(ctx);
                    }

                    leaderboard_nk_score(ctx, i, leaderboard.entries[i]);
                }
                nk_group_end(ctx);
            }
        }
        else
        {
            nk_label(ctx, "You have not completed any deals.", NK_TEXT_ALIGN_MIDDLE | NK_TEXT_ALIGN_CENTERED);
        }

        nk_layout_row_dynamic(ctx, 20, 1);
        nk_spacer(ctx);

        static const float ratio[] = {0.3f, 0.4f, 0.3f};

        nk_layout_row(ctx, NK_DYNAMIC, 30, 3, ratio);
        nk_spacing(ctx, 1);
        if (nk_button_label(ctx, "Back"))
        {
            scene_pop();
        }
        nk_spacing(ctx, 1);
    }
    nk_end(ctx);
}

const Scene LeaderboardScene = {
    .start = leaderboard_start,
    .stop = leaderboard_stop,
    .render = leaderboard_render,
    .popup = 1,
};
