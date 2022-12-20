#include "solitaire.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

int ntlen(void **array)
{
    int i = 0;
    while (array[i] != NULL)
    {
        i++;
    }
    return i;
}

int follows_same_suit(Card first, Card second)
{
    if (first.value == second.value + 1 && first.suit == second.suit)
    {
        return 1;
    }
    return 0;
}

int follows_different_suit(Card first, Card second)
{
    int first_red = first.suit == HEARTS || first.suit == DIAMONDS;
    int second_red = second.suit == HEARTS || second.suit == DIAMONDS;

    if (first.value == second.value + 1 && first_red != second_red)
    {
        return 1;
    }
    return 0;
}

void shuffle_cards(Card deck[MAX_CARDS])
{
    for (int i = MAX_CARDS - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        Card temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}

Solitaire solitaire_create(SolitaireConfig config)
{
    Solitaire game = {
        .config = config,
    };

    const int DECK_SIZE = sizeof(Card) * MAX_CARDS;
    Card *deck = malloc(DECK_SIZE);
    memset(deck, 0, DECK_SIZE);

    for (int i = 0; i < VALUE_MAX; i++)
    {
        for (int j = 0; j < SUIT_MAX; j++)
        {
            Card *card = &deck[i * SUIT_MAX + j];
            card->value = i;
            card->suit = j;
            card->shown = 0;
        }
    }

    if (config.seed == 0)
    {
        config.seed = (int)time(NULL);
    }

    srand(config.seed);

    shuffle_cards(deck);

    int deck_ptr = 0;
    for (int i = 0; i < 7; i++)
    {
        for (int j = 0; j <= i; j++)
        {
            deck[deck_ptr].shown = i == j;
            game.tableu[i][j] = &deck[deck_ptr++];
        }
    }

    // changed
    for (int i = 0; i < MAX_CARDS - 28; i++)
    {
        game.stock[i] = &deck[deck_ptr++];
    }

    return game;
}

int solitaire_free(Solitaire *solitaire)
{
    for (int i = 0; i < solitaire->move_end; i++)
    {
        free(solitaire->moves[i]);
        free(solitaire->moves_data[i]);
        solitaire->moves[i] = NULL;
        solitaire->moves_data[i] = NULL;
    }
    // free moves
    // free moves_data
    free(solitaire->deck);
    solitaire->deck = NULL;
}

int solitaire_is_complete(Solitaire *solitaire)
{
    for (int i = 0; i < SUIT_MAX; i++)
    {
        for (int j = 0; j < VALUE_MAX; j++)
        {
            if (solitaire->foundations[i][j] == NULL)
                return 0;
        }
        if (solitaire->foundations[i][VALUE_MAX] != NULL)
        {
            printf("Invalid foundation: more than %d values\n", VALUE_MAX);
        }
    }
    return 1;
}

int solitaire_push_move(Solitaire *solitaire, Move *move, MoveData *data)
{
    // remove any moves which come after
    for (int i = solitaire->move_index + 1; i < solitaire->move_end; i++)
    {
        free(solitaire->moves[i]);
        free(solitaire->moves_data[i]);
        solitaire->moves[i] = NULL;
        solitaire->moves_data[i] = NULL;
    }

    if (solitaire->move_index >= MAX_MOVES)
    {
        printf("out of moves...\n");
        return 0;
    }

    // add our move to the end
    solitaire->moves[solitaire->move_index] = move;
    solitaire->moves_data[solitaire->move_index] = data;

    solitaire->move_end = solitaire->move_index + 1;

    // return
    return 1;
}

int solitaire_make_move(Solitaire *solitaire, Move move)
{
    // put 'move' onto heap
    Move *moveptr = malloc(sizeof(Move));
    if (!moveptr)
    {
        printf("failed to allocate Move on heap\n");
        return 0;
    }
    memset(moveptr, 0, sizeof(Move));
    moveptr->type = move.type;
    moveptr->from = move.from;
    moveptr->from_x = move.from_x;
    moveptr->from_y = move.from_y;
    moveptr->to = move.to;
    moveptr->to_x = move.to_x;

    // create 'data' on heap
    MoveData *data = malloc(sizeof(MoveData));
    if (!data)
    {
        printf("failed to allocate MoveData on heap\n");
        return 0;
    }
    memset(data, 0, sizeof(MoveData));

    // figure out what 'data' we should set
    switch (move.type)
    {
    case MOVE_CYCLE_STOCK:
        int stock = ntlen(solitaire->stock);
        if (stock == 0)
        {
            data->return_to_stock = 1;
        }
        else
        {
            data->cards_moved = min(stock, solitaire->config.deal_three ? 3 : 1);
        }
        break;
    case MOVE_CARD:
        if (move.from == MOVE_FROM_TABLEU)
        {
            int tableu_len = ntlen(solitaire->tableu[move.from_x]);
            if (move.from_y >= tableu_len)
            {
                printf("from_y should be less than tableu length\n");
                return 0;
            }
            int cards = tableu_len - move.from_y;
            if (move.to == MOVE_TO_FOUNDATION && cards > 1)
            {
                printf("can only move one card to foundation at a time\n");
                return 0;
            }
            data->cards_moved = cards;
            if (move.from_y > 0)
            {
                data->card_revealed = !solitaire->tableu[move.from_x][move.from_y - 1]->shown;
            }
        }
        else
        {
            data->cards_moved = 1;
            if (move.from_y != -1)
            {
                printf("from_y should not be set if moving from foudation or talon\n");
                return 0;
            }
        }
    }

    if (!solitaire_validate_move(solitaire, moveptr, data))
    {
        printf("attempted invalid move!\n");
        return 0;
    }

    // add this move to the end of the moves list
    if (!solitaire_push_move(solitaire, moveptr, data))
    {
        return 0;
    }

    // do move
    return solitaire_redo(solitaire);
}

int solitaire_validate_move(Solitaire *solitaire, Move *move, MoveData *data)
{
    Card *onto = NULL;
    Card *moving = NULL;

    switch (move->from)
    {
    case MOVE_FROM_FOUNDATION:
        int foundation_len = ntlen(solitaire->foundations[move->from_x]);
        if (foundation_len == 0)
        {
            printf("cannot move card from foundation %d - its empty\n", move->from_x);
            return 0;
        }
        moving = solitaire->foundations[move->from_x][foundation_len - 1];
        break;
    case MOVE_FROM_TALON:
        int talon_len = ntlen(solitaire->talon);
        if (talon_len == 0)
        {
            printf("cannot move card from talon - its empty\n");
            return 0;
        }
        moving = solitaire->talon[talon_len - 1];
        break;
    case MOVE_FROM_TABLEU:
        int tableu_len = ntlen(solitaire->tableu[move->from_x]);
        if (tableu_len == 0 || tableu_len < data->cards_moved)
        {
            printf("cannot move card from tableu %d - its empty\n", move->from_x);
            return 0;
        }
        moving = solitaire->tableu[move->from_x][tableu_len - data->cards_moved];
        break;
    }

    switch (move->to)
    {
    case MOVE_TO_FOUNDATION:
        int foundation_len = ntlen(solitaire->foundations[move->to_x]);
        if (foundation_len > 0)
        {
            onto = solitaire->foundations[move->to_x][foundation_len - 1];
        }
        if (onto)
        {
            printf("moving card (%d,%d) onto foundation (%d, %d)\n", moving->suit, moving->value, onto->suit, onto->value);
        }

        if (onto && !follows_same_suit(*moving, *onto))
        {
            printf("cannot move card (%d,%d) onto foundation (%d, %d)\n", moving->suit, moving->value, onto->suit, onto->value);
            return 0;
        }
        else if (!onto && moving->value != ACE)
        {
            printf("cannot move card (%d,%d) onto empty foundation\n", moving->suit, moving->value);
            return 0;
        }
        break;
    case MOVE_TO_TABLEU:
        int tableu_len = ntlen(solitaire->tableu[move->to_x]);
        if (tableu_len > 0)
        {
            onto = solitaire->tableu[move->to_x][tableu_len - 1];
        }

        if (onto && !follows_different_suit(*onto, *moving))
        {
            printf("cannot move card (%d,%d) onto tableu card (%d, %d)\n", moving->suit, moving->value, onto->suit, onto->value);
            return 0;
        }
        else if (!onto && moving->value != KING)
        {
            printf("cannot move card (%d,%d) onto empty tableu column\n", moving->suit, moving->value);
            return 0;
        }
        break;
    }

    return 1;
}

int solitaire_undo(Solitaire *solitaire)
{
    if (!solitaire_can_undo(solitaire))
    {
        return 0;
    }

    Move *move = solitaire->moves[solitaire->move_index - 1];
    MoveData *data = solitaire->moves_data[solitaire->move_index - 1];

    if (move->type == MOVE_CYCLE_STOCK)
    {
        if (data->return_to_stock)
        {
            // move all cards back into talon
            int stock_len = ntlen(solitaire->stock);
            for (int i = 0; i < stock_len; i++)
            {
                solitaire->talon[i] = solitaire->stock[stock_len - 1 - i];
                solitaire->talon[i]->shown = 1;
                solitaire->stock[stock_len - 1 - i] = NULL;
            }
        }
        else
        {
            // move data->cards_moved cards from stock into talon
            int talon_len = ntlen(solitaire->talon);
            int stock_len = ntlen(solitaire->stock);
            for (int i = 0; i < data->cards_moved; i++)
            {
                int talon_index = talon_len - i - 1;
                solitaire->stock[stock_len] = solitaire->talon[talon_index];
                solitaire->stock[stock_len]->shown = 1;
                solitaire->talon[talon_index] = NULL;
                stock_len++;
            }
        }
    }
    else
    {
        Card **from_cards = NULL;
        Card **to_cards = NULL;

        if (move->from == MOVE_FROM_TABLEU && data->card_revealed)
        {
            int tableu_len = ntlen(solitaire->tableu[move->from_x]);
            solitaire->tableu[move->from_x][tableu_len - 1]->shown = 0;
            printf("hid card x=%d,y=%d\n", move->from_x, tableu_len - 1);
        }

        switch (move->from)
        {
        case MOVE_FROM_FOUNDATION:
            int foundation_len = ntlen(solitaire->foundations[move->from_x]);
            from_cards = &solitaire->foundations[move->from_x][foundation_len];
            break;
        case MOVE_FROM_TABLEU:
            int tableu_len = ntlen(solitaire->tableu[move->from_x]);
            from_cards = &solitaire->tableu[move->from_x][tableu_len];
            break;
        case MOVE_FROM_TALON:
            int talon_len = ntlen(solitaire->talon);
            from_cards = &solitaire->talon[talon_len];
            break;
        }

        switch (move->to)
        {
        case MOVE_TO_FOUNDATION:
            to_cards = solitaire->foundations[move->to_x];
            break;
        case MOVE_TO_TABLEU:
            to_cards = solitaire->tableu[move->to_x];
            break;
        }

        int to_cards_len = ntlen(to_cards) - data->cards_moved;

        for (int i = 0; i < data->cards_moved; i++)
        {
            from_cards[i] = to_cards[i + to_cards_len];
            to_cards[i + to_cards_len] = NULL;
        }
    }

    solitaire->move_index--;
}

int solitaire_redo(Solitaire *solitaire)
{
    if (!solitaire_can_redo(solitaire))
    {
        return 0;
    }

    Move *move = solitaire->moves[solitaire->move_index];
    MoveData *data = solitaire->moves_data[solitaire->move_index];

    if (move->type == MOVE_CYCLE_STOCK)
    {
        if (data->return_to_stock)
        {
            // move all cards back into stock
            int talon_len = ntlen(solitaire->talon);
            for (int i = 0; i < talon_len; i++)
            {
                solitaire->stock[i] = solitaire->talon[talon_len - 1 - i];
                solitaire->stock[i]->shown = 0;
                solitaire->talon[talon_len - 1 - i] = NULL;
            }
        }
        else
        {
            // move data->cards_moved cards from stock into talon
            int talon_len = ntlen(solitaire->talon);
            int stock_len = ntlen(solitaire->stock);
            for (int i = 0; i < data->cards_moved; i++)
            {
                int stock_index = stock_len - i - 1;
                solitaire->talon[talon_len] = solitaire->stock[stock_index];
                solitaire->talon[talon_len]->shown = 1;
                solitaire->stock[stock_index] = NULL;
                talon_len++;
            }
        }
    }
    else
    {
        Card **from_cards = NULL;
        Card **to_cards = NULL;

        switch (move->from)
        {
        case MOVE_FROM_FOUNDATION:
            int foundation_len = ntlen(solitaire->foundations[move->from_x]);
            from_cards = &solitaire->foundations[move->from_x][foundation_len - 1];
            break;
        case MOVE_FROM_TABLEU:
            int tableu_len = ntlen(solitaire->tableu[move->from_x]);
            from_cards = &solitaire->tableu[move->from_x][tableu_len - data->cards_moved];
            break;
        case MOVE_FROM_TALON:
            int talon_len = ntlen(solitaire->talon);
            from_cards = &solitaire->talon[talon_len - 1];
            break;
        }

        switch (move->to)
        {
        case MOVE_TO_FOUNDATION:
            to_cards = solitaire->foundations[move->to_x];
            break;
        case MOVE_TO_TABLEU:
            to_cards = solitaire->tableu[move->to_x];
            break;
        }

        int to_cards_len = ntlen(to_cards);

        for (int i = 0; i < data->cards_moved; i++)
        {
            to_cards[i + to_cards_len] = from_cards[i];
            from_cards[i] = NULL;
        }

        if (move->from == MOVE_FROM_TABLEU && data->card_revealed)
        {
            int tableu_len = ntlen(solitaire->tableu[move->from_x]);
            solitaire->tableu[move->from_x][tableu_len - 1]->shown = 1;
            printf("revealed card x=%d,y=%d\n", move->from_x, tableu_len - 1);
        }
    }

    solitaire->move_index++;
}

int solitaire_can_undo(Solitaire *solitaire)
{
    return solitaire->move_index > 0;
}

int solitaire_can_redo(Solitaire *solitaire)
{
    return solitaire->move_index < solitaire->move_end;
}
