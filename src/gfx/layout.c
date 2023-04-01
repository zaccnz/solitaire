#include "gfx/layout.h"

#include "gfx/cards.h"
#include "io/pacman.h"
#include "util/util.h"

#include <raylib-nuklear.h>

#define CARDS_HORIZONTAL 8
#define CARDS_VERTICAL 9

#define GAP_HORIZONTAL 6

int width = 800;
int height = 600;

int card_width = 88;
int card_height = 124;

int card_gap_x = 6;

int layout_width = 0;

float card_vertical_spacing = 0.25f;
float card_aspect_ratio = 1.0f / 1.5f;

#define CLOSEST_MULT(number, step) (number / step) * step

void layout_resize()
{
    Assets *assets = pacman_get_current_assets(ASSET_CARDS);

    width = GetScreenWidth();
    height = GetScreenHeight();

    card_aspect_ratio = (float)assets->cards[0].width / (float)assets->cards[0].height;

    card_width = min(width / CARDS_HORIZONTAL, (height * card_aspect_ratio) / CARDS_VERTICAL);
    card_width = CLOSEST_MULT(card_width, 5);
    card_height = card_width / card_aspect_ratio;
    card_gap_x = CLOSEST_MULT(card_width * 0.1, 2);

    layout_width = (card_width + card_gap_x) * 7 - card_gap_x;

    cards_invalidate_all();
}

void layout_pack_changed()
{
    TexturePack *pack = pacman_get_current(ASSET_CARDS);
    card_vertical_spacing = pack->card_vertical_spacing;

    Assets *assets = pacman_get_current_assets(ASSET_CARDS);
    card_aspect_ratio = (float)assets->cards[0].width / (float)assets->cards[0].height;
    card_height = card_width / card_aspect_ratio;
}

void layout_cardsize(int *width, int *height)
{
    *width = card_width;
    *height = card_height;
}

void layout_position_score(CalcOut *out)
{
    *out = (CalcOut){
        .x = (width - layout_width) / 2,
        .y = 10,
        .width = layout_width,
        .height = 50,
    };
}

void layout_position_foundation(CalcOut *out, int index)
{
    *out = (CalcOut){
        .x = (width - layout_width) / 2,
        .y = 70,
        .width = card_width,
        .height = card_height,
    };

    out->x += index * (card_width + card_gap_x);
}

void layout_position_tableu(CalcOut *out, Coordinate index)
{
    *out = (CalcOut){
        .x = (width - layout_width) / 2,
        .y = 90 + (card_height),
        .width = card_width,
        .height = card_height,
    };

    out->x += index.x * (card_width + card_gap_x);
    out->y += index.y * (float)card_height * card_vertical_spacing;
}

void layout_position_talon(CalcOut *out, int index)
{
    *out = (CalcOut){
        .x = (width + layout_width) / 2 - (card_width * 2 + card_gap_x) - ((index - 1) * (float)card_width * 0.5),
        .y = 70,
        .width = card_width,
        .height = card_height,
    };
}

void layout_position_stock(CalcOut *out)
{
    *out = (CalcOut){
        .x = (width + layout_width) / 2 - card_width,
        .y = 70,
        .width = card_width,
        .height = card_height,
    };
}

void layout_position_controls(CalcOut *out)
{
    int ctrl_pad = 10;
    int ctrl_height = 80;
    if (layout_width < 80 * 5)
    {
        ctrl_height = 60;
    }
    if (layout_width < 60 * 5)
    {
        ctrl_height = 40;
    }

    *out = (CalcOut){
        .x = (width - layout_width) / 2,
        .y = height - ctrl_height - ctrl_pad,
        .width = layout_width,
        .height = ctrl_height,
    };
}

void layout_position_action(CalcOut *out)
{
    int action_width = min(width - 20, 300);
    int action_height = min(height - 40, 250);

    *out = (CalcOut){
        .x = (width - action_width) / 2,
        .y = (height - action_height) / 2,
        .width = action_width,
        .height = action_height,
    };
}

void layout_calculate(LayoutPosition pos, void *data, CalcOut *out)
{
    switch (pos)
    {
    case LAYOUT_SCORE:
        layout_position_score(out);
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
    case LAYOUT_CONTROLS:
        layout_position_controls(out);
        break;
    case LAYOUT_ACTION:
        layout_position_action(out);
        break;
    default:
        break;
    }
}

Rectangle layout_calcout_to_rayrect(CalcOut rect)
{
    return (Rectangle){
        .x = rect.x,
        .y = rect.y,
        .width = rect.width,
        .height = rect.height,
    };
}

struct nk_rect layout_calcout_to_nkrect(CalcOut rect)
{
    return (struct nk_rect){
        .x = rect.x,
        .y = rect.y,
        .w = rect.width,
        .h = rect.height,
    };
}