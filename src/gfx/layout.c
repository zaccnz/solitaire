#include "gfx/layout.h"

int width = 800;
int height = 600;

int card_width = 88;
int card_height = 124;

void layout_resize(int w, int h)
{
    width = w;
    height = h;
    // update card size
}

void layout_cardsize(int *width, int *height)
{
    *width = card_width;
    *height = card_height;
}

void layout_position_foundation(CalcOut *out, int index)
{
    *out = (CalcOut){
        .x = 800 / 2 - (float)(card_width * 3.5) + index * card_width,
        .y = 10,
        .width = card_width,
        .height = card_height,
    };
}

void layout_position_tableu(CalcOut *out, Coordinate index)
{
    *out = (CalcOut){
        .x = 800 / 2 - (float)(card_width * 3.5),
        .y = 450 / 2 - (float)(card_height * 1.5) + 100,
        .width = card_width,
        .height = card_height,
    };

    out->x += index.x * card_width;
    // todo: get from texture pack
    out->y += index.y * (float)card_height * 0.25f;
}

void layout_position_talon(CalcOut *out, int index)
{
    *out = (CalcOut){
        .x = 800 / 2 + (float)(card_width * 2) - (index * (float)card_width * 0.5),
        .y = 10,
        .width = card_width,
        .height = card_height,
    };
}

void layout_position_stock(CalcOut *out)
{
    *out = (CalcOut){
        .x = 800 / 2 + (float)(card_width * 3.5) - card_width,
        .y = 10,
        .width = card_width,
        .height = card_height,
    };
}

void layout_calculate(LayoutPosition pos, void *data, CalcOut *out)
{
    switch (pos)
    {
    case LAYOUT_FOUNDATION:
        layout_position_foundation(out, *(int *)data);
        break;
    case LAYOUT_TABLEU:
        layout_position_tableu(out, *(Coordinate *)data);
        break;
    case LAYOUT_TALON:
        layout_position_talon(out, *(int *)data);
        break;
    case LAYOUT_STOCK:
        layout_position_stock(out);
        break;
    default:
        break;
    }
}