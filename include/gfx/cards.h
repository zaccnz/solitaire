/*
 * handles card rendering, movement and animation
 */
#pragma once

#include "gfx/animator.h"
#include "solitaire.h"

#include <raylib.h>

typedef enum SPRITEFLAGS
{
    FLAGS_NONE = 0,
    FLAGS_FOUNDATION = 1 << 0,
    FLAGS_TABLEU = 1 << 1,
    FLAGS_TALON = 1 << 2,
    FLAGS_STOCK = 1 << 3,
    FLAGS_HITBOX = 1 << 4,
    FLAGS_ANIMATING = 1 << 5,
    FLAGS_REVEALED = 1 << 6,
    FLAGS_INVALIDATED = 1 << 7,
} SpriteFlags;

typedef struct CardSprite
{
    /* gfx */
    int x, y;
    int zindex;
    Rectangle hitbox;

    /* animation */
    AnimationPointer animPtr;

    /* data */
    Suit suit;
    Value value;
    int pile;  /* index of the pile card in*/
    int index; /* the cards index in pile */
    SpriteFlags flags;
} CardSprite;

extern CardSprite cards[MAX_CARDS];
extern int was_deal_stock;

void cards_init();
void cards_free();
void cards_update(Solitaire *solitaire, int background);
void cards_render(Solitaire *solitaire);

void card_place_with_hitbox(CardSprite *card, Card *next, float card_vertical_spacing);
void cards_invalidate_all();

void cards_animate_deal(Solitaire *solitaire);
void cards_animate_move(Solitaire *solitaire, Move move, MoveData data, int undo);
void cards_animate_reveal(Solitaire *solitaire, int tableu_x, int tableu_y);