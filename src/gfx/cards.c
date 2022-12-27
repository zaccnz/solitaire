#include "gfx/cards.h"

#include "gfx/animated.h"
#include "gfx/layout.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>
#include <signal.h>

// todo: move into io/textures
#define CARD_WIDTH 88
#define CARD_HEIGHT 124
#define CARD_COLUMNS 5
Texture2D back, clubs, hearts, spades, diamonds;

#define CLICK_TIME 0.2f

typedef enum SPRITEFLAGS
{
    FLAGS_NONE = 0,
    FLAGS_FOUNDATION = 1 << 0,
    FLAGS_TABLEU = 1 << 1,
    FLAGS_TALON = 1 << 2,
    FLAGS_STOCK = 1 << 3,
    FLAGS_HITBOX = 1 << 4,
    FLAGS_ANIMATING = 1 << 5,
    FLAGS_INVISIBLE = 1 << 6,
    FLAGS_ANIMATING_BOTTOM = 1 << 7,
} SpriteFlags;

typedef struct CardSprite
{
    /* gfx */
    int x, y;
    Rectangle frame;
    Rectangle hitbox;
    Texture2D *texture;

    /* animation */
    AnimationPointer animPtr;

    /* data */
    Suit suit;
    Value value;
    int pile;  /* index of the pile card in*/
    int index; /* the cards index in pile */
    SpriteFlags flags;
} CardSprite;

typedef struct DragAndDrop
{
    CardSprite *cards[VALUE_MAX];
    MoveFrom from;
    int from_idx;
    int count;
} DragAndDrop;

CardSprite cards[VALUE_MAX * SUIT_MAX];
CardSprite back_sprite;
DragAndDrop *dnd = NULL;
float held = 0;

int sv_to_index(Suit suit, Value value)
{
    return suit * VALUE_MAX + value;
}

int card_to_index(Card *card)
{
    return sv_to_index(card->suit, card->value);
}

// NOTE: next will not have been updated yet...
void card_place_with_hitbox(CardSprite *card, int shown, Card *next)
{
    CalcOut out;
    LayoutPosition pos = LAYOUT_NONE;
    int hitbox = 0;
    float hitbox_height_percentage = 1.0;
    Coordinate coord = {
        .x = card->pile,
        .y = card->index,
    };

    void *data;

    if (card->flags & FLAGS_FOUNDATION)
    {
        pos = LAYOUT_FOUNDATION;
        data = &card->pile;
        hitbox = next == NULL;
    }
    if (card->flags & FLAGS_TABLEU)
    {
        pos = LAYOUT_TABLEU;
        data = &coord;
        hitbox = 1;
        // todo: load from texture pack
        hitbox_height_percentage = next == NULL ? 1.0 : 0.25;
    }
    if (card->flags & FLAGS_TALON)
    {
        pos = LAYOUT_TALON;
        data = &card->index;
        hitbox = next == NULL;
    }
    if (card->flags & FLAGS_STOCK)
    {
        pos = LAYOUT_STOCK;
        data = NULL;
        hitbox = next == NULL;
    }

    hitbox = hitbox && shown;

    layout_calculate(pos, data, &out);

    if (!(card->flags & FLAGS_ANIMATING))
    {
        card->x = out.x;
        card->y = out.y;
    }

    if (hitbox)
    {
        card->flags |= FLAGS_HITBOX;
        card->hitbox.x = out.x;
        card->hitbox.y = out.y;
        card->hitbox.width = out.width;
        card->hitbox.height = (float)out.height * hitbox_height_percentage;
    }
}

void cards_init()
{
    // todo: move into texture thingy
    back = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Back - Top Down 88x124.png");
    clubs = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Clubs - Top Down 88x124.png");
    hearts = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Hearts - Top Down 88x124.png");
    spades = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Spades - Top Down 88x124.png");
    diamonds = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Diamonds - Top Down 88x124.png");

    // setup cards as invisible
    for (int i = 0; i < SUIT_MAX; i++)
    {
        for (int j = 0; j < VALUE_MAX; j++)
        {
            CardSprite *sprite = &cards[i * VALUE_MAX + j];
            sprite->x = 0;
            sprite->y = 0;
            sprite->suit = i;
            sprite->value = j;
            sprite->flags = FLAGS_NONE;
            sprite->animPtr.index = -1;
            sprite->animPtr.generation = -1;
            sprite->pile = -1;
            sprite->index = -1;
        }
    }

    back_sprite.x = 0;
    back_sprite.y = 0;
    back_sprite.suit = SUIT_MAX;
    back_sprite.value = VALUE_MAX;
    back_sprite.flags = FLAGS_NONE;
    back_sprite.animPtr.index = -1;
    back_sprite.animPtr.generation = -1;
    back_sprite.index = -1;

    cards_set_textures();
}

void cards_free()
{
    // todo: move into texture thingy
    UnloadTexture(clubs);
    UnloadTexture(hearts);
    UnloadTexture(spades);
    UnloadTexture(diamonds);
    UnloadTexture(back);
}

void cards_set_textures()
{
    // todo: load from texture manager
    for (int i = 0; i < SUIT_MAX; i++)
    {
        for (int j = 0; j < VALUE_MAX; j++)
        {
            CardSprite *sprite = &cards[i * VALUE_MAX + j];
            sprite->frame.width = CARD_WIDTH;
            sprite->frame.height = CARD_HEIGHT;

            switch (sprite->suit)
            {
            case CLUBS:
                sprite->texture = &clubs;
                break;
            case HEARTS:
                sprite->texture = &hearts;
                break;
            case SPADES:
                sprite->texture = &spades;
                break;
            case DIAMONDS:
                sprite->texture = &diamonds;
                break;
            default:
                printf("invalid suit %d\n", sprite->suit);
                break;
            }

            int x = sprite->value % CARD_COLUMNS;
            int y = sprite->value / CARD_COLUMNS;

            sprite->frame.x = x * CARD_WIDTH;
            sprite->frame.y = y * CARD_HEIGHT;
        }
    }

    back_sprite.frame.width = CARD_WIDTH;
    back_sprite.frame.height = CARD_HEIGHT;
    back_sprite.frame.x = 0;
    back_sprite.frame.y = 0;
    back_sprite.texture = &back;
}

int card_check_hit(Vector2 pos, SpriteFlags flags_mask)
{
    for (int i = 0; i < SUIT_MAX * VALUE_MAX; i++)
    {
        if (!(flags_mask & cards[i].flags))
        {
            continue;
        }

        if (!(FLAGS_HITBOX & cards[i].flags))
        {
            continue;
        }

        if (CheckCollisionPointRec(pos, cards[i].hitbox))
        {
            printf("hit card\n");
            return i;
        }
    }
    return -1;
}

void cards_update_sprites(Solitaire *solitaire)
{
    for (int i = 0; i < SUIT_MAX; i++)
    {
        int foundation_len = ntlen((void **)solitaire->foundations[i]);
        for (int j = 0; j < foundation_len; j++)
        {
            Card *card = solitaire->foundations[i][j];
            Card *next = solitaire->foundations[i][j + 1];
            CardSprite *sprite = &cards[card_to_index(card)];
            sprite->flags = FLAGS_FOUNDATION;
            sprite->index = j;
            sprite->pile = i;
            card_place_with_hitbox(sprite, card->shown, next);
        }
    }
    for (int i = 0; i < 7; i++)
    {
        int tableu_len = ntlen((void **)solitaire->tableu[i]);
        for (int j = 0; j < tableu_len; j++)
        {
            Card *card = solitaire->tableu[i][j];
            Card *next = solitaire->tableu[i][j + 1];
            CardSprite *sprite = &cards[card_to_index(card)];
            sprite->flags = FLAGS_TABLEU;
            sprite->index = j;
            sprite->pile = i;
            card_place_with_hitbox(sprite, card->shown, next);
        }
    }
    int talon_len = ntlen((void **)solitaire->talon);
    for (int i = 0; i < talon_len; i++)
    {
        Card *card = solitaire->talon[i];
        Card *next = solitaire->talon[i + 1];
        CardSprite *sprite = &cards[card_to_index(card)];
        sprite->flags = FLAGS_TALON;
        sprite->index = max(-1, min(talon_len - i, 3));
        sprite->pile = -1;
        card_place_with_hitbox(sprite, card->shown, next);
    }
    int stock_len = ntlen((void **)solitaire->stock);
    for (int i = 0; i < stock_len; i++)
    {
        Card *card = solitaire->stock[i];
        Card *next = solitaire->stock[i + 1];
        CardSprite *sprite = &cards[card_to_index(card)];
        sprite->flags = FLAGS_STOCK;
        sprite->index = -1;
        sprite->pile = -1;
        card_place_with_hitbox(sprite, card->shown, next);
    }
}

void cards_update(Solitaire *solitaire)
{
    cards_update_sprites(solitaire);

    // click, drag and drop
    int down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    int released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        held = 0;
    }

    if (down)
    {
        held += GetFrameTime();
    }

    if (!down && !released)
    {
        return;
    }

    if (down && dnd)
    {
        return;
    }

    Vector2 pos = GetMousePosition();
    SpriteFlags mask = FLAGS_NONE;

    if (dnd && held >= CLICK_TIME)
    {
        mask = FLAGS_TABLEU | FLAGS_FOUNDATION;
    }
    else if (held <= CLICK_TIME)
    {
        mask = FLAGS_TABLEU | FLAGS_FOUNDATION | FLAGS_TALON | FLAGS_STOCK;
    }
    else
    {
        mask = FLAGS_TABLEU | FLAGS_FOUNDATION | FLAGS_TALON;
    }

    int hit = card_check_hit(pos, mask);

    if (down)
    {
        printf("down\n");

        if (hit == -1)
        {
            return;
        }

        dnd = malloc(sizeof(DragAndDrop));
        memset(dnd, 0, sizeof(DragAndDrop));

        CardSprite *hit_sprite = &cards[hit];
        if (hit_sprite->flags & FLAGS_FOUNDATION)
        {
            dnd->from = MOVE_FROM_FOUNDATION;
            dnd->from_idx = hit_sprite->pile;
            dnd->count = 1;
            dnd->cards[0] = hit_sprite;
        }
        else if (hit_sprite->flags & FLAGS_TABLEU)
        {
            dnd->from = MOVE_FROM_TABLEU;
            dnd->from_idx = hit_sprite->pile;
            int tableu_len = ntlen((void **)solitaire->tableu[hit_sprite->pile]);
            dnd->count = tableu_len - hit_sprite->index;
            int idx = 0;
            for (int i = hit_sprite->index; i < tableu_len; i++)
            {
                Card *card = solitaire->tableu[hit_sprite->pile][i];
                dnd->cards[idx++] = &cards[card_to_index(card)];
            }
        }
        else if (hit_sprite->flags & FLAGS_TALON)
        {
            dnd->from = MOVE_FROM_TALON;
            dnd->from_idx = -1;
            dnd->count = 1;
            dnd->cards[0] = hit_sprite;
        }
        else
        {
            free(dnd);
            dnd = NULL;
        }
    }
    else if (released)
    {
        if (held < CLICK_TIME)
        {
            printf("up - was clicked\n");
            CalcOut out;
            layout_calculate(LAYOUT_STOCK, NULL, &out);
            Rectangle stock_bounds = {
                .x = out.x,
                .y = out.y,
                .width = out.width,
                .height = out.height,
            };

            // move cards we hit
            if ((hit >= 0 && cards[hit].flags & FLAGS_STOCK) || CheckCollisionPointRec(pos, stock_bounds))
            {
                // deal cards
                Move move = {
                    .type = MOVE_CYCLE_STOCK,
                    .from = MOVE_FROM_NONE,
                    .to = MOVE_TO_NONE,
                    .from_x = -1,
                    .from_y = -1,
                    .to_x = -1,
                };
                solitaire_make_move(solitaire, move);
                return;
            }
            else if (hit >= 0)
            {
                Move move = {0};
                MoveFrom from;
                int from_x = -1;
                int from_y = -1;
                CardSprite *hit_sprite = &cards[hit];
                if (hit_sprite->flags & FLAGS_FOUNDATION)
                {
                    from = MOVE_FROM_FOUNDATION;
                    from_x = hit_sprite->pile;
                }
                else if (hit_sprite->flags & FLAGS_TABLEU)
                {
                    from = MOVE_FROM_TABLEU;
                    from_x = hit_sprite->pile;
                    from_y = hit_sprite->index;
                }
                else if (hit_sprite->flags & FLAGS_TALON)
                {
                    from = MOVE_FROM_TALON;
                }
                if (solitaire_find_move(solitaire, from, from_x, from_y, &move))
                {
                    solitaire_make_move(solitaire, move);
                }
            }
        }
        else
        {
            if (!dnd)
            {
                return;
            }

            printf("up - was dragging\n");
            int from_y = -1;
            if (dnd->from == MOVE_FROM_TABLEU)
            {
                int tableu_from_len = ntlen((void **)solitaire->tableu[dnd->from_idx]);
                from_y = tableu_from_len - dnd->count;
            }

            if (hit >= 0)
            {
                CardSprite *hit_sprite = &cards[hit];
                MoveTo to = MOVE_TO_NONE;

                if (hit_sprite->flags & FLAGS_FOUNDATION)
                {
                    to = MOVE_TO_FOUNDATION;
                }
                else if (hit_sprite->flags & FLAGS_TABLEU)
                {
                    to = MOVE_TO_TABLEU;
                }

                // make move
                Move move = {
                    .type = MOVE_CARD,
                    .from = dnd->from,
                    .to = to,
                    .from_x = dnd->from_idx,
                    .from_y = from_y,
                    .to_x = hit_sprite->pile,
                };
                solitaire_make_move(solitaire, move);
            }
            else
            {
                for (int i = 0; i < SUIT_MAX; i++)
                {
                    CalcOut out;
                    layout_calculate(LAYOUT_FOUNDATION, &i, &out);
                    Rectangle foundation_bounds = {
                        .x = out.x,
                        .y = out.y,
                        .width = out.width,
                        .height = out.height,
                    };

                    printf("checking collision with foundation %d\n", i);

                    if (CheckCollisionPointRec(pos, foundation_bounds))
                    {
                        // make move
                        Move move = {
                            .type = MOVE_CARD,
                            .from = dnd->from,
                            .to = MOVE_TO_FOUNDATION,
                            .from_x = dnd->from_idx,
                            .from_y = from_y,
                            .to_x = i,
                        };
                        solitaire_make_move(solitaire, move);
                        break;
                    }
                }

                for (int i = 0; i < 7; i++)
                {
                    if (ntlen((void **)solitaire->tableu[i]) > 0)
                    {
                        continue;
                    }
                    CalcOut out;
                    Coordinate coord = {
                        .x = i,
                        .y = 0,
                    };
                    layout_calculate(LAYOUT_TABLEU, &coord, &out);
                    Rectangle tableu_bounds = {
                        .x = out.x,
                        .y = out.y,
                        .width = out.width,
                        .height = out.height,
                    };

                    printf("checking collision with tableu %d\n", i);

                    if (CheckCollisionPointRec(pos, tableu_bounds))
                    {
                        // make move
                        Move move = {
                            .type = MOVE_CARD,
                            .from = dnd->from,
                            .to = MOVE_TO_TABLEU,
                            .from_x = dnd->from_idx,
                            .from_y = from_y,
                            .to_x = i,
                        };
                        solitaire_make_move(solitaire, move);
                    }
                }
            }
        }
        if (dnd)
        {
            free(dnd);
            dnd = NULL;
        }
    }
}

void cards_render_card(Card *card)
{

    CardSprite *sprite = &cards[card_to_index(card)];
    Vector2 position = {sprite->x, sprite->y};
    if (!card->shown)
    {
        sprite = &back_sprite;
    }
    DrawTextureRec(*sprite->texture, sprite->frame, position, WHITE);
}

void cards_render(Solitaire *solitaire)
{
    // Render animating (bottom layer)
    CardSprite *animate_top_layer[52];
    int animate_top_layer_count = 0;
    for (int i = 0; i < 52; i++)
    {
        CardSprite *sprite = &cards[i];
        if (!(sprite->flags & FLAGS_ANIMATING))
        {
            continue;
        }

        if (sprite->flags & FLAGS_ANIMATING_BOTTOM)
        {
            Vector2 position = {sprite->x, sprite->y};
            DrawTextureRec(*sprite->texture, sprite->frame, position, WHITE);
        }
        else
        {
            animate_top_layer[animate_top_layer_count++] = sprite;
        }
    }

    // Render foundations
    for (int i = 0; i < SUIT_MAX; i++)
    {
        int foundation_len = ntlen((void **)solitaire->foundations[i]);
        if (dnd && dnd->from == MOVE_FROM_FOUNDATION)
        {
            foundation_len -= dnd->count;
        }
        while (foundation_len > 0)
        {
            Card *card = solitaire->foundations[i][foundation_len - 1];
            CardSprite *sprite = &cards[card_to_index(card)];
            if (sprite->animPtr.index >= 0)
            {
                foundation_len--;
            }
            else
            {
                break;
            }
        }
        foundation_len = max(0, foundation_len);
        if (foundation_len > 0)
        {
            Card *card = solitaire->foundations[i][foundation_len - 1];
            cards_render_card(card);
        }
    }

    // Render talon
    int talon_len = ntlen((void **)solitaire->talon);
    for (int i = 3; i > 0; i--)
    {
        if (dnd && dnd->from == MOVE_FROM_TALON && dnd->count == i)
            continue;
        int index = talon_len - i;
        if (index >= 0)
        {
            Card *card = solitaire->talon[index];
            cards_render_card(card);
        }
    }

    // Render stock
    int stock_len = ntlen((void **)solitaire->stock);
    if (stock_len > 0)
    {
        Card *card = solitaire->stock[stock_len - 1];
        cards_render_card(card);
    }

    // Render talbeu
    for (int i = 0; i < 7; i++)
    {
        int tableu_len = ntlen((void **)solitaire->tableu[i]);
        for (int j = 0; solitaire->tableu[i][j] != NULL; j++)
        {
            if (dnd && dnd->from == MOVE_FROM_TABLEU && dnd->from_idx == i)
            {
                if (tableu_len - dnd->count <= j)
                {
                    break;
                }
            }

            Card *card = solitaire->tableu[i][j];
            cards_render_card(card);
        }
    }

    // Render animating (top layer)
    for (int i = 0; i < animate_top_layer_count; i++)
    {
        CardSprite *sprite = animate_top_layer[i];
        Vector2 position = {sprite->x, sprite->y};
        DrawTextureRec(*sprite->texture, sprite->frame, position, WHITE);
    }

    // Render drag n drop
    if (dnd)
    {
        for (int i = 0; i < dnd->count; i++)
        {
            CardSprite *sprite = dnd->cards[i];
            Vector2 pos = {sprite->x, sprite->y};
            if (sprite->animPtr.index == -1)
            {
                pos = GetMousePosition();
                int w, h;
                layout_cardsize(&w, &h);
                pos.x -= w / 2;
                pos.y -= h / 2;
                pos.y += 0.25 * h * i;
            }
            DrawTextureRec(*sprite->texture, sprite->frame, pos, WHITE);
        }
    }
}

/* Animation Helpers */

typedef struct CardAnimationData
{
    int card;
    int from_x, from_y;
    int to_x, to_y;
} CardAnimationData;

void card_anim_update(float progress, CardAnimationData *data)
{
}

void card_anim_cleanup(int completed, CardAnimationData *data)
{
    free(data);
}

float card_anim_calc_duration(CardAnimationData *data)
{
    return 1.0;
}

/* Animations */

void cards_animate_deal(Solitaire *solitaire)
{
    // play deal animation (call on game created)
}

void cards_animate_move(CardsMove move)
{
    // move a card from a position to a position
    // this should be called from the solitaire.c file
    // when a move is done or undone
}

void cards_animate_reveal(int tableu_x, int tableu_y, int shown)
{
    // flip the card at tableu[tableu_y][tableu_x] to be
    // shown ? visible : invisible
}