/*
 * handles card rendering, movement and animation
 */
#pragma once

typedef struct Cards
{
    void *a;
} Cards;

Cards *cards_init();
void cards_free(Cards *cards);
void cards_update(Cards *cards);
void cards_render(Cards *cards);

void cards_animate_move(Cards *cards, int value, int suit, int destination);