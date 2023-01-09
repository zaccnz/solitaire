/*
 * handles game layout calculations
 *  - card size
 *  - card placement
 *  - ui placement
 */
#pragma once

typedef enum LAYOUTPOSITION
{
    LAYOUT_NONE,

    // game
    LAYOUT_SCORE,
    LAYOUT_FOUNDATION, // data: int (foundation index)
    LAYOUT_TABLEU,     // data: Coordinate (tableu coordinate)
    LAYOUT_TALON,      // data: int (card index)
    LAYOUT_STOCK,

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