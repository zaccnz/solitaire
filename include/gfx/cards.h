/*
 * handles card rendering, movement and animation
 */
#pragma once

#include "solitaire.h"

typedef struct CardsMove
{
    MoveFrom from;
    int from_idx;
    MoveTo to;
    int to_idx;
    int count;
} CardsMove;

void cards_init();
void cards_free();
void cards_set_textures();
void cards_update(Solitaire *solitaire);
void cards_render(Solitaire *solitaire);

void cards_animate_deal(Solitaire *solitaire);
void cards_animate_move(CardsMove move);
void cards_animate_talon(int added);
void cards_animate_reveal(int tableu_x, int tableu_y, int shown);