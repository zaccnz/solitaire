#include "gfx/cards.h"

#include "gfx/animated.h"
#include "gfx/layout.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <raylib.h>
#include <signal.h>

// todo: move into io/textures
#define CARD_WIDTH 88
#define CARD_HEIGHT 124
#define CARD_COLUMNS 5
Texture2D back, clubs, hearts, spades, diamonds;

#define CLICK_TIME 0.2f

typedef enum SPRITEFLAGS
{
    FLAGS_NONE = 0,
    FLAGS_FOUNDATION = 1 << 0,
    FLAGS_TABLEU = 1 << 1,
    FLAGS_TALON = 1 << 2,
    FLAGS_STOCK = 1 << 3,
    FLAGS_HITBOX = 1 << 4,
    FLAGS_ANIMATING = 1 << 5,
    FLAGS_INVISIBLE = 1 << 6,
    FLAGS_REVEALED = 1 << 7,
    FLAGS_INVALIDATED = 1 << 8,
} SpriteFlags;

typedef struct CardSprite
{
    /* gfx */
    int x, y;
    float fx, fy;
    int zindex;
    Rectangle frame;
    Rectangle hitbox;
    Texture2D *texture;

    /* animation */
    AnimationPointer animPtr;

    /* data */
    Suit suit;
    Value value;
    int pile;  /* index of the pile card in*/
    int index; /* the cards index in pile */
    SpriteFlags flags;
} CardSprite;

typedef struct DragAndDrop
{
    CardSprite *cards[VALUE_MAX];
    int zindices[VALUE_MAX];
    MoveFrom from;
    int from_idx;
    int count;
} DragAndDrop;

CardSprite cards[VALUE_MAX * SUIT_MAX];
CardSprite *render_list[VALUE_MAX * SUIT_MAX];
CardSprite back_sprite;
DragAndDrop *dnd = NULL;
float held = 0;

int sv_to_index(Suit suit, Value value)
{
    return suit * VALUE_MAX + value;
}

int card_to_index(Card *card)
{
    return sv_to_index(card->suit, card->value);
}

// NOTE: next will not have been updated yet...
void card_place_with_hitbox(CardSprite *card, Card *next)
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
        // todo: load from texture pack
        hitbox_height_percentage = next == NULL ? 1.0 : 0.25;
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

/* Animation Helpers */

typedef void (*AnimationTargetFunc)(int *to_x, int *to_y);

typedef struct CardAnimationData
{
    int card;
    int from_x, from_y;
    int to_x, to_y;
    float delay;
    float elapsed;
    int zindex;
    AnimationTargetFunc target_func;
} CardAnimationData;

void card_anim_target_mouse(int *to_x, int *to_y)
{
    Vector2 pos = GetMousePosition();
    *to_x = pos.x;
    *to_y = pos.y;
}

int card_anim_update(float progress, CardAnimationData *data)
{
    data->elapsed += progress;
    if (data->elapsed < data->delay)
    {
        return 1;
    }

    int to_x = data->to_x;
    int to_y = data->to_y;

    if (data->target_func)
    {
        data->target_func(&to_x, &to_y);
    }

    float speed = 750.0f;
    CardSprite *sprite = &cards[data->card];
    float dx = (float)(to_x - sprite->x);
    float dy = (float)(to_y - sprite->y);
    // printf("anim dx,dy = %f,%f\n", dx, dy);
    if (abs(dx) < speed * progress)
    {
        sprite->x = to_x;
    }
    else
    {
        sprite->fx += (dx < 0 ? -1.0f : 1.0f) * speed * progress;
        sprite->x = (int)sprite->fx;
    }

    if (abs(dy) < speed * progress)
    {
        sprite->y = to_y;
    }
    else
    {
        sprite->fy += (dy < 0 ? -1.0f : 1.0f) * speed * progress;
        sprite->y = (int)sprite->fy;
    }

    return sprite->x != to_x || sprite->y != to_y;
}

int card_anim_cleanup(int completed, CardAnimationData *data)
{
    CardSprite *sprite = &cards[data->card];
    sprite->animPtr = (AnimationPointer){
        .index = -1,
        .generation = -1,
    };
    sprite->flags &= ~FLAGS_ANIMATING;
    sprite->zindex = data->zindex;
    free(data);
}

#define UPDATE_COUNT (VALUE_MAX * SUIT_MAX)
void cards_animate_to(Solitaire *solitaire, int card, int behind, float delay, int to_x, int to_y, AnimationTargetFunc target_func);

void cards_position_sprites(Solitaire *solitaire, int animate)
{
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
        if (animation_is_behind[update_count])
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

    for (int i = 0; i < update_count; i++)
    {
        update_cards[i]->flags &= ~FLAGS_INVALIDATED;
        card_place_with_hitbox(update_cards[i], update_nexts[i]);
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
        cards_animate_to(solitaire, sv_to_index(card->suit, card->value), animation_is_behind[i], animation_delays[i], to_x, to_y, NULL);
    }
}

void cards_init()
{
    // todo: move into texture thingy
    back = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Back - Top Down 88x124.png");
    clubs = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Clubs - Top Down 88x124.png");
    hearts = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Hearts - Top Down 88x124.png");
    spades = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Spades - Top Down 88x124.png");
    diamonds = LoadTexture("res/SBS_2dPokerPack/Top-Down/Cards/Diamonds - Top Down 88x124.png");

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

    cards_set_textures();
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

    // todo: move into texture thingy
    UnloadTexture(clubs);
    UnloadTexture(hearts);
    UnloadTexture(spades);
    UnloadTexture(diamonds);
    UnloadTexture(back);
}

void cards_set_textures()
{
    // todo: load from texture manager
    for (int i = 0; i < SUIT_MAX; i++)
    {
        for (int j = 0; j < VALUE_MAX; j++)
        {
            CardSprite *sprite = &cards[i * VALUE_MAX + j];
            sprite->frame.width = CARD_WIDTH;
            sprite->frame.height = CARD_HEIGHT;

            switch (sprite->suit)
            {
            case CLUBS:
                sprite->texture = &clubs;
                break;
            case HEARTS:
                sprite->texture = &hearts;
                break;
            case SPADES:
                sprite->texture = &spades;
                break;
            case DIAMONDS:
                sprite->texture = &diamonds;
                break;
            default:
                printf("invalid suit %d\n", sprite->suit);
                break;
            }

            int x = sprite->value % CARD_COLUMNS;
            int y = sprite->value / CARD_COLUMNS;

            sprite->frame.x = x * CARD_WIDTH;
            sprite->frame.y = y * CARD_HEIGHT;
        }
    }

    back_sprite.frame.width = CARD_WIDTH;
    back_sprite.frame.height = CARD_HEIGHT;
    back_sprite.frame.x = 0;
    back_sprite.frame.y = 0;
    back_sprite.texture = &back;
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

void cards_update(Solitaire *solitaire)
{
    cards_position_sprites(solitaire, 1);

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

void cards_render_card(Card *card)
{
    CardSprite *sprite = &cards[card_to_index(card)];
    Vector2 position = {sprite->x, sprite->y};
    if (!card->shown)
    {
        sprite = &back_sprite;
    }
    DrawTextureRec(*sprite->texture, sprite->frame, position, WHITE);
}

void cards_render(Solitaire *solitaire)
{
    // Sort render list by zindex (insertion sort)
    for (int i = 1; i < SUIT_MAX * VALUE_MAX; i++)
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
    for (int i = 0; i < SUIT_MAX * VALUE_MAX; i++)
    {
        CardSprite *sprite = render_list[i];
        if (sprite->flags & FLAGS_INVISIBLE)
        {
            continue;
        }
        Vector2 position = {sprite->x, sprite->y};
        if (!(sprite->flags & FLAGS_REVEALED))
        {
            sprite = &back_sprite;
        }
        DrawTextureRec(*sprite->texture, sprite->frame, position, WHITE);
    }
}

/* Animations */

void cards_animate_to(Solitaire *solitaire, int card, int behind, float delay, int to_x, int to_y, AnimationTargetFunc target_func)
{
    CardSprite *sprite = &cards[card];

    sprite->fx = (float)sprite->x;
    sprite->fy = (float)sprite->y;

    CardAnimationData *data = malloc(sizeof(CardAnimationData));
    data->card = card;
    data->from_x = sprite->x;
    data->from_y = sprite->y;
    data->to_x = to_x;
    data->to_y = to_y;
    data->delay = delay;
    data->elapsed = 0;
    data->target_func = target_func;
    data->zindex = sprite->zindex;

    sprite->zindex = sprite->zindex + (behind ? -52 : 52);

    AnimationConfig config = {
        .on_update = card_anim_update,
        .on_cleanup = card_anim_cleanup,
        .duration = 0,
        .data = data,
    };

    AnimationPointer pointer;

    anim_create(config, &pointer);

    sprite->animPtr = pointer;
    sprite->flags |= FLAGS_ANIMATING;
}

void cards_animate_deal(Solitaire *solitaire)
{
    for (int i = 0; i < VALUE_MAX * SUIT_MAX; i++)
    {
        cards[i].flags |= FLAGS_INVALIDATED;
    }
    cards_position_sprites(solitaire, 0);
    Vector2 positions[VALUE_MAX * SUIT_MAX];
    for (int i = 0; i < VALUE_MAX * SUIT_MAX; i++)
    {
        positions[i].x = cards[i].x;
        positions[i].y = cards[i].y;
        cards[i].x = -CARD_WIDTH;
        cards[i].y = -CARD_HEIGHT;
    }

    // play deal animation (call on game created)
    for (int i = 0; i < VALUE_MAX * SUIT_MAX; i++)
    {
        float delay = 0.0f;
        delay += 0.4f - cards[i].index * 0.2f;
        cards_animate_to(solitaire, i, 0, delay, positions[i].x, positions[i].y, NULL);
    }
}

void cards_place_and_animate()
{
}

void cards_animate_move_card(Solitaire *solitaire, Move move, MoveData data, int undo)
{
    Card **cards_moved;

    if (undo)
    {
        switch (move.to)
        {
        case MOVE_TO_FOUNDATION:
        {
            int foundation_len = ntlen(solitaire->foundations[move.to_x]);
            cards_moved = &solitaire->foundations[move.to_x][foundation_len - data.cards_moved];
            if (foundation_len - data.cards_moved > 0)
            {
                Card *card = solitaire->foundations[move.to_x][foundation_len - data.cards_moved - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        case MOVE_TO_TABLEU:
        {
            int tableu_len = ntlen(solitaire->tableu[move.to_x]);
            cards_moved = &solitaire->tableu[move.to_x][tableu_len - data.cards_moved];
            if (tableu_len - data.cards_moved > 0)
            {
                Card *card = solitaire->tableu[move.to_x][tableu_len - data.cards_moved - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        default:
            break;
        }
        switch (move.from)
        {
        case MOVE_FROM_FOUNDATION:
        {
            int foundation_len = ntlen(solitaire->foundations[move.from_x]);
            if (foundation_len > 0)
            {
                Card *card = solitaire->foundations[move.from_x][foundation_len - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        case MOVE_FROM_TABLEU:
        {
            int tableu_len = ntlen(solitaire->tableu[move.from_x]);
            if (tableu_len > 0)
            {
                Card *card = solitaire->tableu[move.from_x][tableu_len - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        case MOVE_FROM_TALON:
        {
            int talon_len = ntlen(solitaire->talon);
            for (int i = max(talon_len - 6, 0); i < talon_len; i++)
            {
                Card *card = solitaire->talon[i];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        default:
            break;
        }
    }
    else
    {
        switch (move.from)
        {
        case MOVE_FROM_FOUNDATION:
        {
            int foundation_len = ntlen(solitaire->foundations[move.from_x]);
            cards_moved = &solitaire->foundations[move.from_x][foundation_len - data.cards_moved];
            if (foundation_len - data.cards_moved > 0)
            {
                Card *card = solitaire->foundations[move.from_x][foundation_len - data.cards_moved - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        case MOVE_FROM_TABLEU:
        {
            int tableu_len = ntlen(solitaire->tableu[move.from_x]);
            cards_moved = &solitaire->tableu[move.from_x][tableu_len - data.cards_moved];
            if (tableu_len - data.cards_moved > 0)
            {
                Card *card = solitaire->tableu[move.from_x][tableu_len - data.cards_moved - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        case MOVE_FROM_TALON:
        {
            int talon_len = ntlen(solitaire->talon);
            cards_moved = &solitaire->talon[talon_len - data.cards_moved];
            for (int i = max(talon_len - data.cards_moved - 3, 0); i < talon_len - data.cards_moved; i++)
            {
                Card *card = solitaire->talon[i];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        default:
            break;
        }
        switch (move.to)
        {
        case MOVE_TO_FOUNDATION:
        {
            int foundation_len = ntlen(solitaire->foundations[move.to_x]);
            if (foundation_len > 0)
            {
                Card *card = solitaire->foundations[move.to_x][foundation_len - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        case MOVE_TO_TABLEU:
        {
            int tableu_len = ntlen(solitaire->tableu[move.to_x]);
            if (tableu_len > 0)
            {
                Card *card = solitaire->tableu[move.to_x][tableu_len - 1];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            break;
        }
        default:
            break;
        }
    }

    for (int i = 0; i < data.cards_moved; i++)
    {
        Card *card = cards_moved[i];
        CardSprite *sprite = &cards[card_to_index(card)];

        sprite->flags |= FLAGS_REVEALED | FLAGS_INVALIDATED;
    }
}

void cards_animate_move_cycle_stock(Solitaire *solitaire, Move move, MoveData data, int undo)
{
    // move cards back
    Card **from_cards;
    Card **to_cards;
    int from_len;
    int to_len;
    if (data.return_to_stock ^ undo)
    {
        from_cards = solitaire->talon;
        from_len = ntlen(solitaire->talon);
        to_cards = solitaire->stock;
        to_len = ntlen(solitaire->stock);
    }
    else
    {
        from_cards = solitaire->stock;
        from_len = ntlen(solitaire->stock);
        to_cards = solitaire->talon;
        to_len = ntlen(solitaire->talon);
    }

    if (data.return_to_stock)
    {
        for (int i = 0; i < from_len; i++)
        {
            Card *card = from_cards[i];
            CardSprite *sprite = &cards[card_to_index(card)];
            if (undo)
            {
                sprite->flags |= FLAGS_INVALIDATED | FLAGS_REVEALED;
            }
            else
            {
                sprite->flags |= FLAGS_INVALIDATED;
                sprite->flags &= ~FLAGS_REVEALED;
            }
        }
    }
    else
    {
        if (undo)
        {
            for (int i = max(0, to_len - 3); i < to_len; i++)
            {
                Card *card = to_cards[i];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            for (int i = 0; i < from_len; i++)
            {
                Card *card = from_cards[i];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
        }
        else
        {
            for (int i = max(0, from_len - 3); i < from_len; i++)
            {
                Card *card = from_cards[i];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
            for (int i = 0; i < to_len; i++)
            {
                Card *card = to_cards[i];
                CardSprite *sprite = &cards[card_to_index(card)];
                sprite->flags |= FLAGS_INVALIDATED;
            }
        }
    }
}

/*
 * This function is called in solitaire.c by solitaire_undo and solitaire_redo
 * the call occurs before the card has been moved
 */
void cards_animate_move(Solitaire *solitaire, Move move, MoveData data, int undo)
{
    switch (move.type)
    {
    case MOVE_CARD:
    {
        cards_animate_move_card(solitaire, move, data, undo);
        break;
    }
    case MOVE_CYCLE_STOCK:
    {
        cards_animate_move_cycle_stock(solitaire, move, data, undo);
        break;
    }
    }
}

void cards_animate_reveal(Solitaire *solitaire, int tableu_x, int tableu_y)
{
    Card *card = solitaire->tableu[tableu_x][tableu_y];
    CardSprite *sprite = &cards[card_to_index(card)];

    if (card->shown)
    {
        sprite->flags |= FLAGS_REVEALED;
        card_place_with_hitbox(sprite, NULL);
        // start animation to show
    }
    else
    {
        sprite->flags &= ~FLAGS_REVEALED;
        card_place_with_hitbox(sprite, solitaire->tableu[tableu_x][tableu_y + 1]);
        // start animation to hide
    }
}