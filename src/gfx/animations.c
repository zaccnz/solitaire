#include "gfx/animations.h"

#include "gfx/animator.h"
#include "gfx/cards.h"
#include "gfx/layout.h"
#include "io/config.h"
#include "io/pacman.h"
#include "sfx/audio.h"
#include "util/unitbezier.h"
#include "util/util.h"

#include <raylib.h>
#include <stdlib.h>
#include <math.h>

typedef void (*GameEndPlaceCard)(int index, int *card, Vector2 *position, float *delay);

UnitBezier card_move_bezier;

void animations_init()
{
    card_move_bezier = unit_bezier_new(0.55, -0.15, 0.55, 1.15);
}

/* Animation Helpers */
int animation_update(float progress, CardAnimationData *data)
{
    float delay_proportion = data->delay / data->duration;
    if (progress <= delay_proportion)
    {
        return 1;
    }
    float t = (progress - delay_proportion) / (1.0 - delay_proportion);

    CardSprite *sprite = &cards[data->card];
    float fx = data->from_x;
    float fy = data->from_y;
    float tx = data->to_x;
    float ty = data->to_y;

    float ub = unit_bezier_solve(card_move_bezier, t, UNIT_BEZIER_EPSILON);

    float x = ((1.0 - ub) * fx) + (ub * tx);
    float y = ((1.0 - ub) * fy) + (ub * ty);

    sprite->x = x;
    sprite->y = y;

    return 1;
}

int animation_update_reveal(float progress, CardAnimationData *data)
{
    data->progress = progress;

    return 1;
}

int animation_cleanup(int completed, CardAnimationData *data)
{
    CardSprite *sprite = &cards[data->card];
    sprite->animPtr = (AnimationPointer){
        .index = -1,
        .generation = -1,
    };
    sprite->flags &= ~FLAGS_ANIMATING;
    if (!data->reveal)
    {
        sprite->zindex = data->zindex;
    }
    else
    {
        sprite->flags |= FLAGS_INVALIDATED;
    }
    sprite->x = data->to_x;
    sprite->y = data->to_y;
    free(data);

    return 1;
}

/* Animations */

#define CARD_MOVE_SPEED 750 // PIXELS/SECOND

float animation_calc_length(Solitaire *solitaire, CardAnimationData *data)
{
    int dx = data->to_x - data->from_x;
    int dy = data->to_y - data->from_y;
    return sqrtf(dx * dx + dy * dy) / CARD_MOVE_SPEED;
}

void animation_move_card_to(Solitaire *solitaire, int card, int behind, float delay, int to_x, int to_y)
{
    CardSprite *sprite = &cards[card];

    if (sprite->x == to_x && sprite->y == to_y)
    {
        return;
    }

    if (!config.animations)
    {
        sprite->x = to_x;
        sprite->y = to_y;
        return;
    }

    CardAnimationData *data = malloc(sizeof(CardAnimationData));
    data->card = card;
    data->reveal = 0;
    data->from_x = sprite->x;
    data->from_y = sprite->y;
    data->to_x = to_x;
    data->to_y = to_y;
    data->delay = delay;
    data->progress = 0;
    data->zindex = sprite->zindex;

    sprite->zindex = sprite->zindex + (behind ? 0 : 52);

    float animation_length = animation_calc_length(solitaire, data);
    data->duration = delay + animation_length;

    AnimationConfig config = {
        .on_update = (AnimationUpdate)animation_update,
        .on_cleanup = (AnimationCleanup)animation_cleanup,
        .duration = delay + animation_length,
        .data = data,
    };

    anim_create(config, &sprite->animPtr);

    sprite->flags |= FLAGS_ANIMATING;
}

void animation_deal(Solitaire *solitaire)
{
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    int width, height;
    layout_cardsize(&width, &height);

    for (int i = 0; i < MAX_CARDS; i++)
    {
        cards[i].flags |= FLAGS_INVALIDATED;
    }
    cards_position_sprites(solitaire, 0);
    int positions_x[MAX_CARDS];
    int positions_y[MAX_CARDS];
    for (int i = 0; i < MAX_CARDS; i++)
    {
        positions_x[i] = cards[i].x;
        positions_y[i] = cards[i].y;
        cards[i].x = (sw - width) / 2;
        cards[i].y = sh + height;
    }

    // play deal animation (call on game created)
    for (int i = 0; i < MAX_CARDS; i++)
    {
        float delay = 0.0f;
        if (cards[i].flags & FLAGS_TABLEU)
        {
            delay = cards[i].index * 0.2f + cards[i].pile * 0.025f + 0.2f;
        }
        else if (cards[i].flags & FLAGS_STOCK)
        {
            delay = (cards[i].zindex / 6) * 0.05f;
        }
        audio_play_sfx_delay(SFX_DEAL_CARD, delay);
        animation_move_card_to(solitaire, i, 0, delay, positions_x[i], positions_y[i]);
    }
}

void animation_move_card(Solitaire *solitaire, Move move, MoveData data, int undo)
{
    Card **cards_moved;

    if (undo)
    {
        switch (move.to)
        {
        case MOVE_TO_FOUNDATION:
        {
            int foundation_len = ntlen(solitaire->foundations[move.to_x]);
            cards_moved = &solitaire->foundations[move.to_x][foundation_len - data.cards_moved];
            if (foundation_len - data.cards_moved > 0)
            {
                Card *card = solitaire->foundations[move.to_x][foundation_len - data.cards_moved - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        case MOVE_TO_TABLEU:
        {
            int tableu_len = ntlen(solitaire->tableu[move.to_x]);
            cards_moved = &solitaire->tableu[move.to_x][tableu_len - data.cards_moved];
            if (tableu_len - data.cards_moved > 0)
            {
                Card *card = solitaire->tableu[move.to_x][tableu_len - data.cards_moved - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        default:
            break;
        }
        switch (move.from)
        {
        case MOVE_FROM_FOUNDATION:
        {
            int foundation_len = ntlen(solitaire->foundations[move.from_x]);
            if (foundation_len > 0)
            {
                Card *card = solitaire->foundations[move.from_x][foundation_len - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        case MOVE_FROM_TABLEU:
        {
            int tableu_len = ntlen(solitaire->tableu[move.from_x]);
            if (tableu_len > 0)
            {
                Card *card = solitaire->tableu[move.from_x][tableu_len - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        case MOVE_FROM_TALON:
        {
            int talon_len = ntlen(solitaire->talon);
            for (int i = max(talon_len - 6, 0); i < talon_len; i++)
            {
                Card *card = solitaire->talon[i];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        default:
            break;
        }
    }
    else
    {
        switch (move.from)
        {
        case MOVE_FROM_FOUNDATION:
        {
            int foundation_len = ntlen(solitaire->foundations[move.from_x]);
            cards_moved = &solitaire->foundations[move.from_x][foundation_len - data.cards_moved];
            if (foundation_len - data.cards_moved > 0)
            {
                Card *card = solitaire->foundations[move.from_x][foundation_len - data.cards_moved - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        case MOVE_FROM_TABLEU:
        {
            int tableu_len = ntlen(solitaire->tableu[move.from_x]);
            cards_moved = &solitaire->tableu[move.from_x][tableu_len - data.cards_moved];
            if (tableu_len - data.cards_moved > 0)
            {
                Card *card = solitaire->tableu[move.from_x][tableu_len - data.cards_moved - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        case MOVE_FROM_TALON:
        {
            int talon_len = ntlen(solitaire->talon);
            cards_moved = &solitaire->talon[talon_len - data.cards_moved];
            for (int i = max(talon_len - data.cards_moved - 3, 0); i < talon_len - data.cards_moved; i++)
            {
                Card *card = solitaire->talon[i];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        default:
            break;
        }
        switch (move.to)
        {
        case MOVE_TO_FOUNDATION:
        {
            int foundation_len = ntlen(solitaire->foundations[move.to_x]);
            if (foundation_len > 0)
            {
                Card *card = solitaire->foundations[move.to_x][foundation_len - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        case MOVE_TO_TABLEU:
        {
            int tableu_len = ntlen(solitaire->tableu[move.to_x]);
            if (tableu_len > 0)
            {
                Card *card = solitaire->tableu[move.to_x][tableu_len - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        default:
            break;
        }
    }

    for (int i = 0; i < data.cards_moved; i++)
    {
        Card *card = cards_moved[i];
        CardSprite *sprite = &cards[card_to_index(card)];

        sprite->flags |= FLAGS_REVEALED | FLAGS_INVALIDATED;
    }
}

void animation_move_cycle_stock(Solitaire *solitaire, Move move, MoveData data, int undo)
{
    // move cards back
    Card **from_cards;
    Card **to_cards;
    int from_len;
    int to_len;
    if (data.return_to_stock ^ undo)
    {
        from_cards = solitaire->talon;
        from_len = ntlen(solitaire->talon);
        to_cards = solitaire->stock;
        to_len = ntlen(solitaire->stock);
    }
    else
    {
        from_cards = solitaire->stock;
        from_len = ntlen(solitaire->stock);
        to_cards = solitaire->talon;
        to_len = ntlen(solitaire->talon);
    }

    if (data.return_to_stock)
    {
        for (int i = 0; i < from_len; i++)
        {
            Card *card = from_cards[i];
            CardSprite *sprite = &cards[card_to_index(card)];
            if (undo)
            {
                sprite->flags |= FLAGS_INVALIDATED | FLAGS_REVEALED;
            }
            else
            {
                sprite->flags |= FLAGS_INVALIDATED;
                sprite->flags &= ~FLAGS_REVEALED;
            }
        }
    }
    else
    {
        if (undo)
        {
            for (int i = max(0, to_len - 3); i < to_len; i++)
            {
                Card *card = to_cards[i];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            for (int i = 0; i < from_len; i++)
            {
                Card *card = from_cards[i];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
        }
        else
        {
            for (int i = max(0, from_len - 3); i < from_len; i++)
            {
                Card *card = from_cards[i];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            for (int i = 0; i < to_len; i++)
            {
                Card *card = to_cards[i];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
        }
    }
}

void animation_reveal(Solitaire *solitaire, int tableu_x, int tableu_y)
{
    Card *card = solitaire->tableu[tableu_x][tableu_y];
    CardSprite *sprite = &cards[card_to_index(card)];

    int reveal = 0;

    // note: if a card is revealed, there will be no next card.  if a card is hidden,
    // it will have no hitbox.  therefore we can pass '0.0' as card_vertical_spacing
    if (card->shown)
    {
        sprite->flags |= FLAGS_REVEALED;
        cards_place_with_hitbox(sprite, NULL, 0.0);
        reveal = 1;
    }
    else
    {
        sprite->flags &= ~FLAGS_REVEALED;
        cards_place_with_hitbox(sprite, solitaire->tableu[tableu_x][tableu_y + 1], 0.0);
        reveal = -1;
    }

    CardAnimationData *data = malloc(sizeof(CardAnimationData));
    data->card = card_to_index(card);
    data->reveal = reveal;
    data->from_x = sprite->x;
    data->from_y = sprite->y;
    data->to_x = sprite->x;
    data->to_y = sprite->y;
    data->delay = 0;
    data->progress = 0.0;
    data->zindex = sprite->zindex;
    data->duration = 0.3;

    AnimationConfig config = {
        .on_update = (AnimationUpdate)animation_update_reveal,
        .on_cleanup = (AnimationCleanup)animation_cleanup,
        .duration = data->duration,
        .data = data,
    };

    anim_create(config, &sprite->animPtr);

    sprite->flags |= FLAGS_ANIMATING;
}

/*
 * This function is called in solitaire.c by solitaire_undo and solitaire_redo
 * the call occurs before the card has been moved
 */
void animation_move(Solitaire *solitaire, Move move, MoveData data, int undo)
{
    switch (move.type)
    {
    case MOVE_CARD:
    {
        animation_move_card(solitaire, move, data, undo);
        break;
    }
    case MOVE_CYCLE_STOCK:
    {
        animation_move_cycle_stock(solitaire, move, data, undo);
        was_deal_stock = !undo;
        break;
    }
    }
}

/* STANDALONE ANIMATION HELPERS */
int standalone_render(float progress, StandaloneAnimationData *data)
{
    if (data->elapsed < data->delay)
    {
        return 1;
    }
    Assets *card_assets = pacman_get_current_assets(ASSET_CARDS);
    Texture tex = card_assets->cards[data->card];

    Rectangle source = {
        .x = 0,
        .y = 0,
        .width = tex.width,
        .height = tex.height,
    };

    int cw, ch;
    layout_cardsize(&cw, &ch);

    Rectangle dest = {
        .x = data->x,
        .y = data->y,
        .width = cw,
        .height = ch,
    };
    DrawTexturePro(tex, source, dest, (Vector2){.x = 0.0f, .y = 0.0f}, 0.0f, WHITE);
    return 1;
}

int standalone_resize(int sw, int sh, StandaloneAnimationData *data)
{
    data->sw = sw;
    data->sh = sh;
    return 0;
}

int standalone_cleanup(int complete, StandaloneAnimationData *data)
{
    free(data);
    return 1;
}

/* STANDALONE ANIMATION - MAIN MENU */
int standalone_update_menu(float progress, StandaloneAnimationData *data)
{
    int cw, ch;
    layout_cardsize(&cw, &ch);

    data->elapsed += progress;

    int card_pad = cw * 0.2f;
    data->x = (data->sw - 4 * (cw + card_pad)) / 2 + (data->index * (cw + card_pad));
    float fy = sin(data->elapsed) * 50.0 + (float)(data->sh - (ch * 3)) / 2.0;
    data->y = fy;

    return 1;
}

void animation_main_menu()
{
    int sw = GetScreenWidth(), sh = GetScreenHeight();
    int cw, ch;
    layout_cardsize(&cw, &ch);

    int card_pad = cw * 0.2f;

    for (int i = 0; i < SUIT_MAX; i++)
    {
        int card = i * VALUE_MAX + ACE;

        StandaloneAnimationData *data = malloc(sizeof(StandaloneAnimationData));
        data->card = card;
        data->x = (sw - 4 * (cw + card_pad)) / 2 + (i * (cw + card_pad));
        data->y = (sh - (ch * 3)) / 2;
        data->elapsed = i * 0.5f;
        data->delay = 0;
        data->sw = sw;
        data->sh = sh;
        data->index = i;

        AnimationConfig cfg = {
            .on_update = (AnimationUpdate)standalone_update_menu,
            .on_render = (AnimationRender)standalone_render,
            .on_resize = (AnimationResize)standalone_resize,
            .on_cleanup = (AnimationCleanup)standalone_cleanup,
            .duration = 0,
            .data = data,
        };

        anim_create(cfg, NULL);
    }
}

/* STANDALONE ANIMATION - GAME END ANIMATIONS */
#define GAME_END_ANIMATION_COUNT 1

typedef struct GameEndData1
{
} GameEndData1;

Vector2 game_end_1_position(int index)
{
    int suit = index % SUIT_MAX;

    CalcOut origin;
    layout_calculate(LAYOUT_FOUNDATION, &suit, &origin);

    return (Vector2){0.0, 0.0};
}

int game_end_1_update(float progress, StandaloneAnimationData *data)
{
    data->elapsed += progress;
    return 1;
}

int game_end_1_resize(int width, int height, StandaloneAnimationData *data)
{
    Vector2 pos = game_end_1_position(data->index);
    data->x = pos.x;
    data->y = pos.y;
    data->sw = width;
    data->sh = height;
    return 0;
}

void game_end_1_place_card(int index, int *card, Vector2 *position, float *delay)
{
    int suit = index % SUIT_MAX;
    int suit_card = index / SUIT_MAX;

    *card = suit * VALUE_MAX + KING;
    *delay = suit_card * 0.1;
    *position = game_end_1_position(index);
}

const AnimationUpdate GAME_END_ANIMATIONS[GAME_END_ANIMATION_COUNT] = {
    (AnimationUpdate)game_end_1_update,
};
const AnimationResize GAME_END_ANIMATIONS_RESIZE[GAME_END_ANIMATION_COUNT] = {
    (AnimationResize)game_end_1_resize,
};
const GameEndPlaceCard GAME_END_ANIMATIONS_PLACE_CARD[GAME_END_ANIMATION_COUNT] = {
    (GameEndPlaceCard)game_end_1_place_card,
};
const size_t GAME_END_ANIMATIONS_EXTRA_DATA[GAME_END_ANIMATION_COUNT] = {
    sizeof(GameEndData1),
};
const int GAME_END_ANIMATIONS_CARD_COUNT[GAME_END_ANIMATION_COUNT] = {
    MAX_CARDS * 24,
};

void animation_game_end()
{
    int sw = GetScreenWidth(), sh = GetScreenHeight();

    int animation = rand() % GAME_END_ANIMATION_COUNT;

    int cards = GAME_END_ANIMATIONS_CARD_COUNT[animation];
    GameEndPlaceCard place_func = GAME_END_ANIMATIONS_PLACE_CARD[animation];
    AnimationUpdate update_func = GAME_END_ANIMATIONS[animation];
    AnimationResize resize_func = GAME_END_ANIMATIONS_RESIZE[animation];
    if (!resize_func)
    {
        resize_func = (AnimationResize)standalone_resize;
    }

    for (int i = 0; i < cards; i++)
    {
        Vector2 pos;
        StandaloneAnimationData *data = malloc(sizeof(StandaloneAnimationData) + GAME_END_ANIMATIONS_EXTRA_DATA[i]);
        place_func(i, &data->card, &pos, &data->delay);
        data->x = pos.x;
        data->y = pos.y;
        data->elapsed = 0;
        data->sw = sw;
        data->sh = sh;
        data->index = i;

        AnimationConfig cfg = {
            .on_update = update_func,
            .on_render = (AnimationRender)standalone_render,
            .on_resize = resize_func,
            .on_cleanup = (AnimationCleanup)standalone_cleanup,
            .duration = 0,
            .data = data,
        };

        anim_create(cfg, NULL);
    }
}