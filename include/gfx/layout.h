/*
 * handles game layout calculations
 *  - card size
 *  - card placement
 *  - ui placement
 */
#pragma once

#include <raylib.h>

typedef enum LAYOUTPOSITION
{
    LAYOUT_NONE,

    // game
    LAYOUT_SCORE,      // data: NULL
    LAYOUT_FOUNDATION, // data: int (foundation index)
    LAYOUT_TABLEU,     // data: Coordinate (tableu coordinate)
    LAYOUT_TALON,      // data: int (card index)
    LAYOUT_STOCK,      // data: NULL
    LAYOUT_CONTROLS,   // data: NULL
    LAYOUT_ACTION,     // data: NULL

    LAYOUT_MAX,
} LayoutPosition;

typedef struct CalcOut
{
    int x;
    int y;
    int width;
    int height;
} CalcOut;

typedef struct Coordinate
{
    int x;
    int y;
} Coordinate;

void layout_resize();
void layout_pack_changed();
void layout_cardsize(int *width, int *height);
void layout_calculate(LayoutPosition pos, void *data, CalcOut *out);
Rectangle layout_calcout_to_rayrect(CalcOut rect);
struct nk_rect layout_calcout_to_nkrect(CalcOut rect);