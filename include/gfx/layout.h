/*
 * handles game layout calculations
 *  - card size
 *  - card placement
 *  - ui placement
 */
#pragma once

typedef enum LAYOUTPOSITIONING
{
    LAYOUT_NONE,

    // menu
    LAYOUT_HEADER,
    LAYOUT_BUTTONS_CENTER,
    LAYOUT_BUTTON_BOTTOM_LEFT,
    LAYOUT_BUTTON_BOTTOM_CENTER,
    LAYOUT_BUTTON_BOTTOM_RIGHT,

    // game
    LAYOUT_SCORE,
    LAYOUT_FOUNDATION,
    LAYOUT_TABLEU,
    LAYOUT_TALON,
    LAYOUT_STOCK,

    LAYOUT_MAX,
} LayoutPositioning;

typedef enum LAYOUTSIZING
{
} LayoutSizing;

typedef struct LayoutPos
{
    void *null;
} LayoutPos;

typedef struct CalcOut
{
    void *null;
} CalcOut;

void layout_cardsize();
void layout_calculate(LayoutPos pos, int width, int height, CalcOut *out);