#include "gfx/cards.h"

#include "gfx/animations.h"
#include "gfx/layout.h"
#include "io/config.h"
#include "io/pacman.h"
#include "util/util.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define CLICK_TIME 0.2f

typedef struct DragAndDrop
{
    CardSprite *cards[VALUE_MAX];
    int zindices[VALUE_MAX];
    MoveFrom from;
    int from_idx;
    int count;
} DragAndDrop;

CardSprite cards[MAX_CARDS];
CardSprite *render_list[MAX_CARDS];
CardSprite back_sprite;
DragAndDrop *dnd = NULL;
int was_deal_stock = 0;
float held = 0;

int sv_to_index(Suit suit, Value value)
{
    return suit * VALUE_MAX + value;
}

int card_to_index(Card *card)
{
    return sv_to_index(card->suit, card->value);
}

#define UPDATE_COUNT (VALUE_MAX * SUIT_MAX)

void cards_position_sprites(Solitaire *solitaire, int animate)
{
    TexturePack *current_pack = pacman_get_current(TEXTURE_CARDS);
    int update_count = 0;
    CardSprite *update_cards[UPDATE_COUNT];
    Card *update_nexts[UPDATE_COUNT];
    Vector2 animation_origins[UPDATE_COUNT];
    int animation_is_behind[UPDATE_COUNT] = {0};
    float animation_delays[UPDATE_COUNT] = {0.0f};

    for (int i = 0; i < SUIT_MAX; i++)
    {
        int foundation_len = ntlen((void **)solitaire->foundations[i]);
        for (int j = 0; j < foundation_len; j++)
        {
            Card *card = solitaire->foundations[i][j];
            CardSprite *sprite = &cards[card_to_index(card)];
            if (!(sprite->flags & FLAGS_INVALIDATED))
            {
                continue;
            }
            update_cards[update_count] = sprite;
            update_nexts[update_count] = solitaire->foundations[i][j + 1];
            animation_origins[update_count].x = sprite->x;
            animation_origins[update_count].y = sprite->y;
            update_count++;

            sprite->flags = FLAGS_FOUNDATION;
            if (card->shown)
            {
                sprite->flags |= FLAGS_REVEALED;
            }
            sprite->index = j;
            sprite->pile = i;
            sprite->zindex = j;
        }
    }
    for (int i = 0; i < 7; i++)
    {
        int tableu_len = ntlen((void **)solitaire->tableu[i]);
        for (int j = 0; j < tableu_len; j++)
        {
            Card *card = solitaire->tableu[i][j];
            CardSprite *sprite = &cards[card_to_index(card)];
            if (!(sprite->flags & FLAGS_INVALIDATED))
            {
                continue;
            }
            update_cards[update_count] = sprite;
            update_nexts[update_count] = solitaire->tableu[i][j + 1];
            animation_origins[update_count].x = sprite->x;
            animation_origins[update_count].y = sprite->y;
            update_count++;

            sprite->flags = FLAGS_TABLEU;
            if (card->shown)
            {
                sprite->flags |= FLAGS_REVEALED;
            }
            sprite->index = j;
            sprite->pile = i;
            sprite->zindex = j;
        }
    }
    int talon_len = ntlen((void **)solitaire->talon);
    for (int i = 0; i < talon_len; i++)
    {
        Card *card = solitaire->talon[i];
        CardSprite *sprite = &cards[card_to_index(card)];
        if (!(sprite->flags & FLAGS_INVALIDATED))
        {
            continue;
        }
        update_cards[update_count] = sprite;
        update_nexts[update_count] = solitaire->talon[i + 1];
        animation_origins[update_count].x = sprite->x;
        animation_origins[update_count].y = sprite->y;
        animation_is_behind[update_count] = talon_len - i > 3;
        if (!animation_is_behind[update_count] && was_deal_stock)
        {
            animation_delays[update_count] = 0.4f - min(talon_len - i - 1, 3) * 0.2f;
        }
        update_count++;

        sprite->flags = FLAGS_TALON | FLAGS_REVEALED;
        sprite->index = max(-1, min(talon_len - i, 3));
        sprite->pile = -1;
        sprite->zindex = i;
    }
    int stock_len = ntlen((void **)solitaire->stock);
    for (int i = 0; i < stock_len; i++)
    {
        Card *card = solitaire->stock[i];
        CardSprite *sprite = &cards[card_to_index(card)];
        if (!(sprite->flags & FLAGS_INVALIDATED))
        {
            continue;
        }
        update_cards[update_count] = sprite;
        update_nexts[update_count] = solitaire->stock[i + 1];
        animation_origins[update_count].x = sprite->x;
        animation_origins[update_count].y = sprite->y;
        update_count++;

        sprite->flags = FLAGS_STOCK;
        sprite->index = -1;
        sprite->pile = -1;
        sprite->zindex = i;
    }

    was_deal_stock = 0;

    for (int i = 0; i < update_count; i++)
    {
        update_cards[i]->flags &= ~FLAGS_INVALIDATED;
        card_place_with_hitbox(update_cards[i], update_nexts[i], current_pack->card_vertical_spacing);
    }

    if (!animate)
    {
        return;
    }

    for (int i = 0; i < update_count; i++)
    {
        CardSprite *card = update_cards[i];
        int to_x = card->x;
        int to_y = card->y;
        card->x = animation_origins[i].x;
        card->y = animation_origins[i].y;
        animation_move_card_to(solitaire, sv_to_index(card->suit, card->value), animation_is_behind[i], animation_delays[i], to_x, to_y);
    }
}

void cards_init()
{
    // setup cards as invisible
    for (int i = 0; i < SUIT_MAX; i++)
    {
        for (int j = 0; j < VALUE_MAX; j++)
        {
            int index = i * VALUE_MAX + j;
            CardSprite *sprite = &cards[index];
            sprite->x = 0;
            sprite->y = 0;
            sprite->suit = i;
            sprite->value = j;
            sprite->flags = FLAGS_NONE;
            sprite->animPtr.index = -1;
            sprite->animPtr.generation = -1;
            sprite->pile = -1;
            sprite->index = -1;

            render_list[index] = sprite;
        }
    }

    back_sprite.x = 0;
    back_sprite.y = 0;
    back_sprite.suit = SUIT_MAX;
    back_sprite.value = VALUE_MAX;
    back_sprite.flags = FLAGS_NONE;
    back_sprite.animPtr.index = -1;
    back_sprite.animPtr.generation = -1;
    back_sprite.index = -1;

    animations_init();
}

void cards_free()
{
    for (int i = 0; i < SUIT_MAX * VALUE_MAX; i++)
    {
        CardSprite *sprite = cards + i;
        if (sprite->animPtr.index >= 0)
        {
            anim_cancel(sprite->animPtr);
        }
    }
}

int card_check_hit(Vector2 pos, SpriteFlags flags_mask)
{
    for (int i = 0; i < SUIT_MAX * VALUE_MAX; i++)
    {
        if (!(flags_mask & cards[i].flags))
        {
            continue;
        }

        if (!(FLAGS_HITBOX & cards[i].flags))
        {
            continue;
        }

        if (CheckCollisionPointRec(pos, cards[i].hitbox))
        {
            return i;
        }
    }
    return -1;
}

void cards_on_click(Solitaire *solitaire, CardSprite *target)
{
    CalcOut out;
    layout_calculate(LAYOUT_STOCK, NULL, &out);
    Rectangle stock_bounds = {
        .x = out.x,
        .y = out.y,
        .width = out.width,
        .height = out.height,
    };

    // move cards we hit
    if (CheckCollisionPointRec(GetMousePosition(), stock_bounds))
    {
        // deal cards
        Move move = {
            .type = MOVE_CYCLE_STOCK,
            .from = MOVE_FROM_NONE,
            .to = MOVE_TO_NONE,
            .from_x = -1,
            .from_y = -1,
            .to_x = -1,
        };
        solitaire_make_move(solitaire, move);
        return;
    }
    else if (target)
    {
        Move move = {0};
        MoveFrom from;
        int from_x = -1;
        int from_y = -1;
        if (target->flags & FLAGS_FOUNDATION)
        {
            from = MOVE_FROM_FOUNDATION;
            from_x = target->pile;
        }
        else if (target->flags & FLAGS_TABLEU)
        {
            from = MOVE_FROM_TABLEU;
            from_x = target->pile;
            from_y = target->index;
        }
        else if (target->flags & FLAGS_TALON)
        {
            from = MOVE_FROM_TALON;
        }
        if (solitaire_find_move(solitaire, from, from_x, from_y, &move))
        {
            solitaire_make_move(solitaire, move);
        }
    }
}

void cards_on_begin_drag(Solitaire *solitaire, CardSprite *target)
{
    if (target == NULL)
    {
        return;
    }

    dnd = malloc(sizeof(DragAndDrop));
    memset(dnd, 0, sizeof(DragAndDrop));

    if (target->flags & FLAGS_FOUNDATION)
    {
        dnd->from = MOVE_FROM_FOUNDATION;
        dnd->from_idx = target->pile;
        dnd->count = 1;
        dnd->cards[0] = target;
    }
    else if (target->flags & FLAGS_TABLEU)
    {
        dnd->from = MOVE_FROM_TABLEU;
        dnd->from_idx = target->pile;
        int tableu_len = ntlen((void **)solitaire->tableu[target->pile]);
        dnd->count = tableu_len - target->index;
        int idx = 0;
        for (int i = target->index; i < tableu_len; i++)
        {
            Card *card = solitaire->tableu[target->pile][i];
            dnd->cards[idx++] = &cards[card_to_index(card)];
        }
    }
    else if (target->flags & FLAGS_TALON)
    {
        dnd->from = MOVE_FROM_TALON;
        dnd->from_idx = -1;
        dnd->count = 1;
        dnd->cards[0] = target;
    }
    else
    {
        free(dnd);
        dnd = NULL;
    }

    for (int i = 0; i < dnd->count; i++)
    {
        dnd->zindices[i] = dnd->cards[i]->zindex;
        dnd->cards[i]->zindex += 52;
    }
}

void cards_on_drop(Solitaire *solitaire, CardSprite *target)
{
    if (!dnd)
    {
        return;
    }

    int from_y = -1;
    if (dnd->from == MOVE_FROM_TABLEU)
    {
        int tableu_from_len = ntlen((void **)solitaire->tableu[dnd->from_idx]);
        from_y = tableu_from_len - dnd->count;
    }

    if (target)
    {
        MoveTo to = MOVE_TO_NONE;

        if (target->flags & FLAGS_FOUNDATION)
        {
            to = MOVE_TO_FOUNDATION;
        }
        else if (target->flags & FLAGS_TABLEU)
        {
            to = MOVE_TO_TABLEU;
        }

        if (dnd->from != to || dnd->from_idx != target->pile)
        {
            // make move
            Move move = {
                .type = MOVE_CARD,
                .from = dnd->from,
                .to = to,
                .from_x = dnd->from_idx,
                .from_y = from_y,
                .to_x = target->pile,
            };
            if (solitaire_make_move(solitaire, move))
            {
                return;
            }
        }
    }

    Vector2 pos = GetMousePosition();
    int collision = 0;
    MoveTo to;
    int to_x;
    for (int i = 0; i < SUIT_MAX; i++)
    {
        CalcOut out;
        layout_calculate(LAYOUT_FOUNDATION, &i, &out);
        Rectangle foundation_bounds = {
            .x = out.x,
            .y = out.y,
            .width = out.width,
            .height = out.height,
        };
        if (dnd->from == MOVE_FROM_FOUNDATION && dnd->from_idx == i)
        {
            continue;
        }

        if (CheckCollisionPointRec(pos, foundation_bounds))
        {
            collision = 1;
            to = MOVE_TO_FOUNDATION;
            to_x = i;
            break;
        }
    }

    for (int i = 0; i < 7; i++)
    {
        if (ntlen((void **)solitaire->tableu[i]) > 0)
        {
            continue;
        }
        if (dnd->from == MOVE_FROM_TABLEU && dnd->from_idx == i)
        {
            continue;
        }
        CalcOut out;
        Coordinate coord = {
            .x = i,
            .y = 0,
        };
        layout_calculate(LAYOUT_TABLEU, &coord, &out);
        Rectangle tableu_bounds = {
            .x = out.x,
            .y = out.y,
            .width = out.width,
            .height = out.height,
        };

        if (CheckCollisionPointRec(pos, tableu_bounds))
        {
            collision = 1;
            to = MOVE_TO_TABLEU;
            to_x = i;
        }
    }

    if (collision)
    {
        Move move = {
            .type = MOVE_CARD,
            .from = dnd->from,
            .to = to,
            .from_x = dnd->from_idx,
            .from_y = from_y,
            .to_x = to_x,
        };
        if (solitaire_make_move(solitaire, move))
        {
            printf("made move \n");
            return;
        }
    }

    for (int i = 0; i < dnd->count; i++)
    {
        CardSprite *card = dnd->cards[i];
        card->zindex = dnd->zindices[i];
        card->flags |= FLAGS_INVALIDATED;
    }
}

void cards_update(Solitaire *solitaire, int background)
{
    cards_position_sprites(solitaire, 1);

    if (background)
    {
        return;
    }

    int down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    int released = IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        held = 0;
    }
    else if (down)
    {
        held += GetFrameTime();
    }

    if (!down && !released)
    {
        return;
    }

    if (down && dnd && dnd->count > 0)
    {
        if (!(dnd->cards[0]->flags & FLAGS_ANIMATING))
        {
            Vector2 pos = GetMousePosition();
            int w, h;
            layout_cardsize(&w, &h);
            pos.x -= w / 2;
            pos.y -= h / 2;
            for (int i = 0; i < dnd->count; i++)
            {
                CardSprite *card = dnd->cards[i];
                card->x = pos.x;
                card->y = pos.y + 0.25 * h * i;
            }
        }
        return;
    }

    Vector2 pos = GetMousePosition();
    SpriteFlags mask = FLAGS_NONE;

    if (dnd && held >= CLICK_TIME)
    {
        mask = FLAGS_TABLEU | FLAGS_FOUNDATION;
    }
    else
    {
        mask = FLAGS_TABLEU | FLAGS_FOUNDATION | FLAGS_TALON;
    }

    int hit = card_check_hit(pos, mask);
    CardSprite *target = NULL;
    if (hit >= 0)
    {
        target = &cards[hit];
    }

    if (down)
    {
        cards_on_begin_drag(solitaire, target);
    }
    else if (released)
    {
        if (held < CLICK_TIME)
        {
            cards_on_click(solitaire, target);
        }
        else
        {
            cards_on_drop(solitaire, target);
        }
        if (dnd)
        {
            if (held < CLICK_TIME)
            {
                for (int i = 0; i < dnd->count; i++)
                {
                    CardSprite *card = dnd->cards[i];
                    card->zindex = dnd->zindices[i];
                    card->flags |= FLAGS_INVALIDATED;
                }
            }
            free(dnd);
            dnd = NULL;
        }
    }
}

void cards_render(Solitaire *solitaire, struct nk_context *ctx)
{
    TexturePack *pack_cards = pacman_get_current(TEXTURE_CARDS);
    TexturePack *pack_backs = pacman_get_current(TEXTURE_BACKS);

    Textures *tex_cards = pacman_get_current_textures(TEXTURE_CARDS);
    Textures *tex_backs = pacman_get_current_textures(TEXTURE_BACKS);

    Rectangle source = {
        .x = 0,
        .y = 0,
        .width = 0,
        .height = 0,
    };

    int w, h;
    layout_cardsize(&w, &h);
    Rectangle dest = {0, 0, w, h};
    Vector2 origin = {0, 0};

    // Sort render list by zindex (insertion sort)
    for (int i = 1; i < MAX_CARDS; i++)
    {
        CardSprite *ptr = render_list[i];
        int j = i - 1;
        while (j >= 0 && render_list[j]->zindex > ptr->zindex)
        {
            render_list[j + 1] = render_list[j];
            j = j - 1;
        }
        render_list[j + 1] = ptr;
    }

    // Render
    for (int i = 0; i < MAX_CARDS; i++)
    {
        CardSprite *sprite = render_list[i];
        int index = sprite->suit * VALUE_MAX + sprite->value;
        Texture texture = tex_cards->cards[index];

        if (!(sprite->flags & FLAGS_REVEALED))
        {
            texture = tex_backs->card_back;
        }

        source.width = texture.width;
        source.height = texture.height;
        dest.x = sprite->x;
        dest.y = sprite->y;
        DrawTexturePro(texture, source, dest, origin, 0.0f, WHITE);
    }

    if (config.debug.render_hitboxes)
    {
        for (int i = 0; i < MAX_CARDS; i++)
        {
            CardSprite *sprite = render_list[i];

            if (!(sprite->flags & FLAGS_HITBOX))
            {
                continue;
            }

            char buffer[256];
            snprintf(buffer, 256, "%s %s", SUITS[sprite->suit], VALUE[sprite->value]);
            DrawText(buffer, sprite->hitbox.x + 2, sprite->hitbox.y + sprite->hitbox.height - 14, 12, RED);
            DrawRectangleLinesEx(sprite->hitbox, 2, BLUE);
        }
    }

    if (config.debug.render_animation_list)
    {
        struct nk_rect animation_list_bounds = nk_rect(
            config.window_size.width - 150,
            config.window_size.height - 200,
            140, 140);

        if (nk_begin(ctx, "animation list", animation_list_bounds, NK_WINDOW_BORDER))
        {
            nk_layout_row_static(ctx, 30, 120, 1);
            for (int i = 0; i < MAX_CARDS; i++)
            {
                CardSprite *sprite = render_list[i];

                if (!(sprite->flags & FLAGS_ANIMATING))
                {
                    continue;
                }

                nk_labelf(ctx, NK_TEXT_LEFT, "%s %s (%.02f)", SUITS[sprite->suit],
                          VALUE[sprite->value], anim_get_duration(sprite->animPtr));
            }
        }
        nk_end(ctx);
    }
}

void card_place_with_hitbox(CardSprite *card, Card *next, float card_vertical_spacing)
{
    CalcOut out;
    LayoutPosition pos = LAYOUT_NONE;
    int hitbox = 0;
    float hitbox_height_percentage = 1.0;
    Coordinate coord = {
        .x = card->pile,
        .y = card->index,
    };

    void *data;

    if (card->flags & FLAGS_FOUNDATION)
    {
        pos = LAYOUT_FOUNDATION;
        data = &card->pile;
        hitbox = next == NULL;
    }
    if (card->flags & FLAGS_TABLEU)
    {
        pos = LAYOUT_TABLEU;
        data = &coord;
        hitbox = 1;
        hitbox_height_percentage = next == NULL ? 1.0 : card_vertical_spacing;
    }
    if (card->flags & FLAGS_TALON)
    {
        pos = LAYOUT_TALON;
        data = &card->index;
        hitbox = next == NULL;
    }
    if (card->flags & FLAGS_STOCK)
    {
        pos = LAYOUT_STOCK;
        data = NULL;
        hitbox = 0;
    }

    hitbox = hitbox && (card->flags & FLAGS_REVEALED);

    layout_calculate(pos, data, &out);

    if (!(card->flags & FLAGS_ANIMATING))
    {
        card->x = out.x;
        card->y = out.y;
    }

    if (hitbox)
    {
        card->flags |= FLAGS_HITBOX;
        card->hitbox.x = out.x;
        card->hitbox.y = out.y;
        card->hitbox.width = out.width;
        card->hitbox.height = (float)out.height * hitbox_height_percentage;
    }
    else
    {
        card->flags &= ~FLAGS_HITBOX;
    }
}

void cards_invalidate_all()
{
    for (int i = 0; i < SUIT_MAX * VALUE_MAX; i++)
    {
        CardSprite *sprite = &cards[i];
        sprite->flags |= FLAGS_INVALIDATED;
    }
}