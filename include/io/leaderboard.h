#pragma once

#include <stdint.h>

#define LEADERBOARD_ENTRY_MAX 10

typedef struct LeaderboardEntry
{
    int seed;
    int score;
    int time;
    int moves;
    int64_t achieved;
} LeaderboardEntry;

typedef struct Leaderboard
{
    int high_score;
    int fastest_time;
    int lowest_moves;

    LeaderboardEntry entries[LEADERBOARD_ENTRY_MAX];
    int entry_count;
} Leaderboard;

extern Leaderboard leaderboard;

void leaderboard_load();
// note: this function will save the leaderboard if something changes
void leaderboard_submit(int seed, int score, int time, int moves);