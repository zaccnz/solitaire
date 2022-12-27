#pragma once

typedef enum SUIT
{
    CLUBS = 0,
    HEARTS,
    SPADES,
    DIAMONDS,
    SUIT_MAX,
} Suit;

typedef enum VALUE
{
    ACE = 0,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING,
    VALUE_MAX,
} Value;

typedef struct Card
{
    Suit suit;
    Value value;
    int shown;
} Card;

int follows_same_suit(Card first, Card second);
int follows_different_suit(Card first, Card second);

#define MAX_CARDS (SUIT_MAX * VALUE_MAX)
#define MAX_MOVES 1024

typedef enum MOVEFROM
{
    MOVE_FROM_NONE,
    MOVE_FROM_FOUNDATION,
    MOVE_FROM_TABLEU,
    MOVE_FROM_TALON,
} MoveFrom;

typedef enum MOVETO
{
    MOVE_TO_NONE,
    MOVE_TO_FOUNDATION,
    MOVE_TO_TABLEU,
} MoveTo;

typedef enum MOVETYPE
{
    MOVE_CYCLE_STOCK,
    MOVE_CARD,
    MOVE_MAX,
} MoveType;

typedef struct Move
{
    MoveType type;
    MoveFrom from;
    MoveTo to;
    int from_x;
    int from_y;
    int to_x;
} Move;

typedef struct MoveData
{
    int cards_moved;     // MOVE_CYCLE_STOCK | MOVE_CARD_*
    int card_revealed;   // MOVE_CARD_TABLEU_*
    int return_to_stock; // MOVE_CYCLE_STOCK
} MoveData;

typedef struct SolitaireConfig
{
    int seed;
    int deal_three;
} SolitaireConfig;

typedef struct Solitaire
{
    SolitaireConfig config;
    Card *deck;

    Card *tableu[7][MAX_CARDS];
    Card *foundations[4][VALUE_MAX + 1];
    Card *talon[MAX_CARDS];
    Card *stock[MAX_CARDS];

    Move *moves[MAX_MOVES];
    MoveData *moves_data[MAX_MOVES];
    int move_index;
    int move_end;
} Solitaire;

// simple to implement
Solitaire solitaire_create(SolitaireConfig config);
int solitaire_free(Solitaire *solitaire);
int solitaire_is_complete(Solitaire *solitaire);

int solitaire_make_move(Solitaire *solitaire, Move move);
int solitaire_can_undo(Solitaire *solitaire);
int solitaire_can_redo(Solitaire *solitaire);
int solitaire_undo(Solitaire *solitaire);
int solitaire_redo(Solitaire *solitaire);

// more difficult
int solitaire_find_move(Solitaire *solitaire, MoveFrom from, int from_x, int from_y, Move *move);
int solitaire_make_trivial_move(Solitaire *solitaire);
int solitaire_is_solvable(Solitaire *solitaire);
int solitaire_is_trivial(Solitaire *solitaire);