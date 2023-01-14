#include "gfx/animations.h"

#include "gfx/animator.h"
#include "gfx/cards.h"
#include "io/config.h"
#include "util/unitbezier.h"

#include <stdlib.h>
#include <math.h>

UnitBezier card_move_bezier;

void animations_init()
{
    card_move_bezier = unit_bezier_new(0.55, -0.15, 0.55, 1.15);
}

/* Animation Helpers */

typedef struct CardAnimationData
{
    int card;
    int from_x, from_y;
    int to_x, to_y;
    float delay;
    float duration;
    int zindex;
} CardAnimationData;

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

int animation_cleanup(int completed, CardAnimationData *data)
{
    CardSprite *sprite = &cards[data->card];
    sprite->animPtr = (AnimationPointer){
        .index = -1,
        .generation = -1,
    };
    sprite->flags &= ~FLAGS_ANIMATING;
    sprite->zindex = data->zindex;
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
    data->from_x = sprite->x;
    data->from_y = sprite->y;
    data->to_x = to_x;
    data->to_y = to_y;
    data->delay = delay;
    data->zindex = sprite->zindex;

    sprite->zindex = sprite->zindex + (behind ? 0 : 52);

    float animation_length = animation_calc_length(solitaire, data);
    data->duration = delay + animation_length;

    AnimationConfig config = {
        .on_update = animation_update,
        .on_cleanup = animation_cleanup,
        .duration = delay + animation_length,
        .data = data,
    };

    anim_create(config, &sprite->animPtr);

    sprite->flags |= FLAGS_ANIMATING;
}

void animation_deal(Solitaire *solitaire)
{
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
        cards[i].x = -width;
        cards[i].y = -height;
    }

    // play deal animation (call on game created)
    for (int i = 0; i < MAX_CARDS; i++)
    {
        float delay = 0.0f;
        delay += cards[i].index * 0.2f;
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

    // note: if a card is revealed, there will be no next card.  if a card is hidden,
    // it will have no hitbox.  therefore we can pass '0.0' as card_vertical_spacing

    if (card->shown)
    {
        sprite->flags |= FLAGS_REVEALED;
        card_place_with_hitbox(sprite, NULL, 0.0);
        // start animation to show
    }
    else
    {
        sprite->flags &= ~FLAGS_REVEALED;
        card_place_with_hitbox(sprite, solitaire->tableu[tableu_x][tableu_y + 1], 0.0);
        // start animation to hide
    }
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
        was_deal_stock = 1;
        break;
    }
    }
}