#include "gfx/layout.h"

#include "gfx/cards.h"
#include "io/pacman.h"
#include "util/util.h"

#include <raylib.h>

#define CARDS_HORIZONTAL 8
#define CARDS_VERTICAL 7

#define GAP_HORIZONTAL 6

int width = 800;
int height = 600;

int card_width = 88;
int card_height = 124;

int card_gap_x = 6;

float card_vertical_spacing = 0.25f;
float card_aspect_ratio = 1.0f / 1.5f;

#define CLOSEST_MULT(number, step) (number / step) * step

void layout_resize()
{
    printf("layout resized\n");

    width = GetScreenWidth();
    height = GetScreenHeight();

    card_width = min(width / CARDS_HORIZONTAL, (height * card_aspect_ratio) / CARDS_VERTICAL);
    card_width = CLOSEST_MULT(card_width, 5);
    card_height = card_width / card_aspect_ratio;
    card_gap_x = CLOSEST_MULT(card_width * 0.1, 2);

    cards_invalidate_all();
}

void layout_pack_changed()
{
    TexturePack *pack = pacman_get_current(TEXTURE_CARDS);
    card_vertical_spacing = pack->card_vertical_spacing;
}

void layout_cardsize(int *width, int *height)
{
    *width = card_width;
    *height = card_height;
}

void layout_position_foundation(CalcOut *out, int index)
{
    int hw = width / 2;
    *out = (CalcOut){
        .x = hw - (float)(card_width + card_gap_x) * 3.5 + index * (card_width + card_gap_x),
        .y = 10,
        .width = card_width,
        .height = card_height,
    };
}

void layout_position_tableu(CalcOut *out, Coordinate index)
{
    int hw = width / 2;
    int hh = height / 2;
    *out = (CalcOut){
        .x = hw - (float)(card_width + card_gap_x) * 3.5f,
        .y = 10 + (float)(card_height * 1.5),
        .width = card_width,
        .height = card_height,
    };

    out->x += index.x * (card_width + card_gap_x);
    out->y += index.y * (float)card_height * card_vertical_spacing;
}

void layout_position_talon(CalcOut *out, int index)
{
    int hw = width / 2;
    *out = (CalcOut){
        .x = hw + (float)(card_width * 2) - (index * (float)card_width * 0.5),
        .y = 10,
        .width = card_width,
        .height = card_height,
    };
}

void layout_position_stock(CalcOut *out)
{
    int hw = width / 2;
    *out = (CalcOut){
        .x = hw + (float)(card_width * 3.5) - card_width,
        .y = 10,
        .width = card_width,
        .height = card_height,
    };
}

void layout_calculate(LayoutPosition pos, void *data, CalcOut *out)
{
    switch (pos)
    {
    case LAYOUT_SCORE:
        break;
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