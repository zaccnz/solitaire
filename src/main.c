#include "gfx/animated.h"
#include "scene.h"
#include "solitaire.h"
#include "scenes/menu.h"

#include <raylib.h>
#include <string.h>
#include <stdlib.h>

#include <rlgl.h>

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include <raylib-nuklear.h>

// todo: replace with TOML

#define CARD_WIDTH 88
#define CARD_HEIGHT 124
#define CARD_COLUMNS 5

typedef struct DragAndDrop
{
    MoveFrom from;
    Card *cards[VALUE_MAX];
    int x;
    int tableu_y;
} DragAndDrop;

Texture2D *get_card_texture(Card *card, Rectangle *frame, Texture2D *back, Texture2D *clubs, Texture2D *hearts, Texture2D *spades, Texture2D *diamonds)
{
    frame->width = CARD_WIDTH;
    frame->height = CARD_HEIGHT;

    Texture2D *texture = NULL;
    if (card->shown)
    {
        switch (card->suit)
        {
        case CLUBS:
            texture = clubs;
            break;
        case HEARTS:
            texture = hearts;
            break;
        case SPADES:
            texture = spades;
            break;
        case DIAMONDS:
            texture = diamonds;
            break;
        }

        int x = card->value % CARD_COLUMNS;
        int y = card->value / CARD_COLUMNS;

        frame->x = x * CARD_WIDTH;
        frame->y = y * CARD_HEIGHT;
    }
    else
    {
        frame->x = 0;
        frame->y = 0;
        texture = back;
    }

    return texture;
}

float rotation = 0.0;

int animation_update_test(float progress, void *data)
{
    rotation = progress;
    printf("animation progress: %.02f\n", progress);
}

int main(void)
{
    Solitaire solitaire = solitaire_create((SolitaireConfig){
        .seed = 0,
        .deal_three = 1,
    });
    DragAndDrop *dnd = NULL;

    // check for existing settings

    // scene_push(&MenuScene);

    int tableu_masks[7] = {-1, -1, -1, -1, -1, -1, -1};
    int foundation_masks[4] = {-1, -1, -1, -1};
    int waste_mask = -1;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "solitaire");

    Texture2D back = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Back - Top Down 88x124.png");
    Texture2D clubs = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Clubs - Top Down 88x124.png");
    Texture2D hearts = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Hearts - Top Down 88x124.png");
    Texture2D spades = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Spades - Top Down 88x124.png");
    Texture2D diamonds = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Diamonds - Top Down 88x124.png");

    Rectangle frame = {
        0.0f,
        0.0f,
        (float)CARD_WIDTH,
        (float)CARD_HEIGHT,
    };

    struct nk_context *ctx = InitNuklear(10);

    while (!WindowShouldClose())
    {
        anim_update();

        if (IsKeyPressed(KEY_R))
        {
            solitaire = solitaire_create((SolitaireConfig){
                .seed = 0,
                .deal_three = 1,
            });

            for (int i = 0; i < 7; i++)
            {
                tableu_masks[i] = -1;
            }
            for (int i = 0; i < 4; i++)
            {
                foundation_masks[i] = -1;
            }
            waste_mask = -1;
        }

        if (IsKeyPressed(KEY_Z))
        {
            solitaire_undo(&solitaire);
        }
        if (IsKeyPressed(KEY_X))
        {
            solitaire_redo(&solitaire);
        }
        if (IsKeyPressed(KEY_A))
        {
            printf("creating animation\n");
            AnimationConfig config = {
                .on_update = animation_update_test,
                .duration = 3.0f,
            };
            anim_create(config, NULL);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            Vector2 pos = GetMousePosition();
            Rectangle stock = {
                .x = 800 / 2 + (float)(CARD_WIDTH * 3.5) - CARD_WIDTH,
                .y = 10,
                .width = CARD_WIDTH,
                .height = CARD_HEIGHT,
            };

            if (CheckCollisionPointRec(pos, stock))
            {
                Move move = {
                    .type = MOVE_CYCLE_STOCK,
                    .from = MOVE_FROM_NONE,
                    .to = MOVE_TO_NONE,
                    .from_x = -1,
                    .from_y = -1,
                    .to_x = -1,
                };
                solitaire_make_move(&solitaire, move);
            }
        }
        else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && dnd == NULL)
        {
            // find mouse target (just a hack for now)
            {
                int start_x = 800 / 2 - (float)(CARD_WIDTH * 3.5);
                int start_y = 450 / 2 - (float)(CARD_HEIGHT * 1.5) + 100;

                int hit = 0;
                int hit_x = 0;
                int hit_y = 0;

                Vector2 pos = GetMousePosition();
                for (int i = 0; i < 7; i++)
                {
                    for (int j = 0; solitaire.tableu[i][j] != NULL; j++)
                    {
                        if (!solitaire.tableu[i][j]->shown)
                        {
                            continue;
                        }
                        int last = solitaire.tableu[i][j + 1] == NULL;
                        int x = start_x + i * CARD_WIDTH;
                        int y = start_y + j * (float)CARD_HEIGHT * 0.25f;
                        Rectangle bbox = {
                            .x = x,
                            .y = y,
                            .width = CARD_WIDTH,
                            .height = last ? CARD_HEIGHT : (float)CARD_HEIGHT * 0.25f,
                        };
                        if (CheckCollisionPointRec(pos, bbox))
                        {
                            hit = 1;
                            hit_x = i;
                            hit_y = j;
                        }
                    }
                }

                // create DND
                if (hit)
                {
                    dnd = malloc(sizeof(DragAndDrop));
                    memset(dnd, 0, sizeof(DragAndDrop));
                    dnd->from = MOVE_FROM_TABLEU;
                    dnd->x = hit_x;
                    dnd->tableu_y = hit_y;
                    int i = 0;
                    for (int j = hit_y; solitaire.tableu[hit_x][j] != NULL; j++)
                    {
                        dnd->cards[i++] = solitaire.tableu[hit_x][j];
                    }

                    tableu_masks[hit_x] = hit_y;
                }
            }
            if (dnd == NULL)
            {
                int hit = 0;
                int hit_x = 0;
                int foundation_len = 0;

                Vector2 pos = GetMousePosition();
                for (int i = 0; i < SUIT_MAX; i++)
                {
                    foundation_len = ntlen(solitaire.foundations[i]);
                    if (foundation_len == 0)
                        continue;
                    Vector2 position = {800 / 2 - (float)(CARD_WIDTH * 3.5) + i * CARD_WIDTH, 10};
                    Rectangle bbox = {
                        .x = position.x,
                        .y = position.y,
                        .width = CARD_WIDTH,
                        .height = CARD_HEIGHT,
                    };
                    if (CheckCollisionPointRec(pos, bbox))
                    {
                        hit = 1;
                        hit_x = i;
                        break;
                    }
                }

                // create DND
                if (hit)
                {

                    dnd = malloc(sizeof(DragAndDrop));
                    memset(dnd, 0, sizeof(DragAndDrop));
                    dnd->from = MOVE_FROM_FOUNDATION;
                    dnd->x = hit_x;
                    dnd->tableu_y = -1;
                    dnd->cards[0] = solitaire.foundations[hit_x][foundation_len - 1];

                    foundation_masks[hit_x] = 1;
                }
            }
            if (dnd == NULL)
            {
                int hit = 0;
                int waste_len = 0;

                Vector2 pos = GetMousePosition();

                Vector2 position = {800 / 2 + (float)(CARD_WIDTH * 1.5), 10};
                Rectangle bbox = {
                    .x = position.x,
                    .y = position.y,
                    .width = CARD_WIDTH,
                    .height = CARD_HEIGHT,
                };

                if (CheckCollisionPointRec(pos, bbox))
                {
                    hit = 1;
                }

                // create DND
                if (hit)
                {
                    waste_len = ntlen(solitaire.talon);

                    dnd = malloc(sizeof(DragAndDrop));
                    memset(dnd, 0, sizeof(DragAndDrop));
                    dnd->from = MOVE_FROM_TALON;
                    dnd->x = -1;
                    dnd->tableu_y = -1;
                    dnd->cards[0] = solitaire.talon[waste_len - 1];

                    waste_mask = 1;
                }
            }
        }
        else if (IsMouseButtonUp(MOUSE_BUTTON_LEFT) && dnd != NULL)
        {
            MoveTo target;
            int hit = 0;
            int hit_x = 0;

            Vector2 pos = GetMousePosition();

            // find drop target (foundations)
            for (int i = 0; i < 4; i++)
            {
                Vector2 position = {800 / 2 - (float)(CARD_WIDTH * 3.5) + i * CARD_WIDTH, 10};
                Rectangle bbox = {
                    .x = position.x,
                    .y = position.y,
                    .width = CARD_WIDTH,
                    .height = CARD_HEIGHT,
                };
                if (CheckCollisionPointRec(pos, bbox))
                {
                    printf("dropping on foundation!\n");
                    target = MOVE_TO_FOUNDATION;
                    hit = 1;
                    hit_x = i;
                }
            }

            // find drop target (tableu)
            if (!hit)
            {
                int start_x = 800 / 2 - (float)(CARD_WIDTH * 3.5);
                int start_y = 450 / 2 - (float)(CARD_HEIGHT * 1.5) + 100;

                for (int i = 0; i < 7; i++)
                {
                    int tableu_len = ntlen(solitaire.tableu[i]);

                    int x = start_x + i * CARD_WIDTH;
                    int y = start_y + tableu_len * (float)CARD_HEIGHT * 0.25f;
                    Rectangle bbox = {
                        .x = x,
                        .y = y,
                        .width = CARD_WIDTH,
                        .height = CARD_HEIGHT,
                    };
                    if (CheckCollisionPointRec(pos, bbox))
                    {
                        target = MOVE_TO_TABLEU;
                        hit = 1;
                        hit_x = i;
                    }
                }
            }

            // create DND
            if (hit && !(dnd->from == target && dnd->x == hit_x))
            {
                int move_from_x = -1;
                int move_from_y = -1;

                switch (dnd->from)
                {
                case MOVE_FROM_FOUNDATION:
                    move_from_x = dnd->x;
                    break;
                case MOVE_FROM_TABLEU:
                    move_from_x = dnd->x;
                    move_from_y = dnd->tableu_y;
                    break;
                case MOVE_FROM_TALON:
                    break;
                }

                Move move = {
                    .type = MOVE_CARD,
                    .from = dnd->from,
                    .to = target,
                    .from_x = move_from_x,
                    .from_y = move_from_y,
                    .to_x = hit_x,
                };
                printf("making move %d %d %d %d %d\n", move.from, move.to, move.from_x, move.from_y, move.to_x);
                solitaire_make_move(&solitaire, move);
            }

            switch (dnd->from)
            {
            case MOVE_FROM_FOUNDATION:
                foundation_masks[dnd->x] = -1;
                break;
            case MOVE_FROM_TABLEU:
                tableu_masks[dnd->x] = -1;
                break;
            case MOVE_FROM_TALON:
                waste_mask = -1;
            }
            free(dnd);
            dnd = NULL;
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

        // scene_update(0.0);
        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (int i = 0; i < SUIT_MAX; i++)
        {
            Vector2 position = {800 / 2 - (float)(CARD_WIDTH * 3.5) + i * CARD_WIDTH, 10};

            int foundation_len = ntlen(solitaire.foundations[i]);
            if (foundation_masks[i] >= 0)
            {
                foundation_len -= foundation_masks[i];
            }
            if (foundation_len == 0)
            {
                DrawText("foundation", position.x, position.y, 10, GRAY);
            }
            else
            {
                Card *card = solitaire.foundations[i][foundation_len - 1];
                Texture2D *texture = get_card_texture(card, &frame, &back, &clubs, &hearts, &spades, &diamonds);
                DrawTextureRec(*texture, frame, position, WHITE);
            }
        }

        int talon_len = ntlen(solitaire.talon);
        for (int i = 3; i > 0; i--)
        {
            if (waste_mask == i)
                continue;
            int index = talon_len - i;
            if (index >= 0)
            {
                Card *card = solitaire.talon[index];

                Vector2 position = {800 / 2 + (float)(CARD_WIDTH * 2) - (i * (float)CARD_WIDTH * 0.5), 10};
                Texture2D *texture = get_card_texture(card, &frame, &back, &clubs, &hearts, &spades, &diamonds);
                DrawTextureRec(*texture, frame, position, WHITE);
            }
        }

        int stock_len = ntlen(solitaire.stock);
        if (stock_len > 0)
        {
            Card card = {
                .suit = 0,
                .value = 0,
                .shown = 0,
            };

            Vector2 position = {800 / 2 + (float)(CARD_WIDTH * 3.5) - CARD_WIDTH, 10};
            Vector2 zero = {-CARD_WIDTH / 2, -CARD_HEIGHT / 2};
            Texture2D *texture = get_card_texture(&card, &frame, &back, &clubs, &hearts, &spades, &diamonds);

            rlPushMatrix();
            printf("rotation: %f\n", 360.0 * rotation);
            rlTranslatef(800 / 2 + (float)(CARD_WIDTH * 3.0), 10.0 + (float)CARD_HEIGHT * 0.5, 0.0);
            rlRotatef(360.0 * rotation, 0.0, 1.0, 0.0);
            DrawTextureRec(*texture, frame, zero, WHITE);
            rlPopMatrix();
        }

        for (int i = 0; i < 7; i++)
        {
            for (int j = 0; solitaire.tableu[i][j] != NULL; j++)
            {
                if (tableu_masks[i] != -1 && j >= tableu_masks[i])
                {
                    break;
                }

                Vector2 position = {800 / 2 - (float)(CARD_WIDTH * 3.5), 450 / 2 - (float)(CARD_HEIGHT * 1.5) + 100};

                position.x += i * CARD_WIDTH;
                position.y += j * (float)CARD_HEIGHT * 0.25f;

                Card *card = solitaire.tableu[i][j];
                Texture2D *texture = get_card_texture(card, &frame, &back, &clubs, &hearts, &spades, &diamonds);
                DrawTextureRec(*texture, frame, position, WHITE);
            }
        }

        // draw held cards
        // scene_render();

        if (dnd)
        {
            Vector2 pos = GetMousePosition();
            pos.x -= CARD_WIDTH / 2;
            pos.y -= CARD_HEIGHT / 2;
            for (int i = 0; dnd->cards[i] != NULL; i++)
            {
                pos.y += (float)CARD_HEIGHT * 0.25f;

                Card *card = dnd->cards[i];
                Texture2D *texture = get_card_texture(card, &frame, &back, &clubs, &hearts, &spades, &diamonds);
                DrawTextureRec(*texture, frame, pos, WHITE);
            }
        }

        // DrawText("this IS a texture!", 360, 370, 10, GRAY);

        if (solitaire_is_complete(&solitaire))
        {
            DrawText("game complete! well done", 360, 370, 30, GRAY);
        }

        DrawText("controls: [r] restart [z] undo [x] redo", 10, 574, 16, GRAY);

        DrawNuklear(ctx);

        EndDrawing();
    }

    anim_release();

    // scene_pop();
    UnloadTexture(clubs);
    UnloadTexture(hearts);
    UnloadTexture(spades);
    UnloadTexture(diamonds);
    UnloadTexture(back);
    UnloadNuklear(ctx);

    CloseWindow();

    solitaire_free(&solitaire);
    return 0;
}