#pragma once

#include "solitaire.h"

void animations_init();

void animation_move_card_to(Solitaire *solitaire, int card, int behind, float delay, int to_x, int to_y);
void animation_deal(Solitaire *solitaire);
void animation_reveal(Solitaire *solitaire, int tableu_x, int tableu_y);
void animation_move(Solitaire *solitaire, Move move, MoveData data, int undo);