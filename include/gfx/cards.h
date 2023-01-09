/*
 * handles card rendering, movement and animation
 */
#pragma once

#include "solitaire.h"

void cards_init();
void cards_free();
void cards_update(Solitaire *solitaire, int background);
void cards_render(Solitaire *solitaire);

void cards_invalidate_all();

void cards_animate_deal(Solitaire *solitaire);
void cards_animate_move(Solitaire *solitaire, Move move, MoveData data, int undo);
void cards_animate_reveal(Solitaire *solitaire, int tableu_x, int tableu_y);