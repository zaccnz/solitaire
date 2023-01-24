#pragma once

#include "solitaire.h"

typedef struct CardAnimationData
{
    int card;
    int from_x, from_y;
    int to_x, to_y;
    float delay;
    float duration;
    int zindex;
} CardAnimationData;

void animations_init();

// in game animations
void animation_move_card_to(Solitaire *solitaire, int card, int behind, float delay, int to_x, int to_y);
void animation_deal(Solitaire *solitaire);
void animation_reveal(Solitaire *solitaire, int tableu_x, int tableu_y);
void animation_move(Solitaire *solitaire, Move move, MoveData data, int undo);

typedef struct StandaloneAnimationData
{
    int card;
    int x, y;
    int index;
    float elapsed;
    int sw, sh;
} StandaloneAnimationData;

// standalone animations
void animation_main_menu();
void animation_game_end();