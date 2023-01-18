/*
 * handles card rendering, movement and animation
 */
#pragma once

#include "gfx/animator.h"
#include "solitaire.h"

#include <raylib.h>
#include <raylib-nuklear.h>

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

#define FLAGS_POSITIONS (FLAGS_FOUNDATION | FLAGS_TABLEU | FLAGS_TALON | FLAGS_STOCK)

typedef struct CardSprite
{
    /* gfx */
    int x, y;
    int zindex;
    Rectangle hitbox;
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
void cards_render(Solitaire *solitaire, struct nk_context *ctx);

void cards_position_sprites(Solitaire *solitaire, int animate);
void cards_place_with_hitbox(CardSprite *card, Card *next, float card_vertical_spacing);
void cards_invalidate_all();