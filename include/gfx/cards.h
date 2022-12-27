/*
 * handles card rendering, movement and animation
 */
#pragma once

#include "solitaire.h"

void cards_init();
void cards_free();
void cards_set_textures();
void cards_update(Solitaire *solitaire);
void cards_render(Solitaire *solitaire);

void cards_animate_deal(Solitaire *solitaire);
void cards_animate_move(Move move, int undo);
void cards_animate_reveal(int tableu_x, int tableu_y, int shown);