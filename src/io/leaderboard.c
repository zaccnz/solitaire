#include "io/leaderboard.h"

#include "util/toml_writer.h"
#include "util/util.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <toml.h>

const Leaderboard DEFAULT_LEADERBOARD = {
    .high_score = -1,
    .fastest_time = -1,
    .lowest_moves = -1,
    .entries = {0},
    .entry_count = 0,
};

Leaderboard leaderboard;

void leaderboard_load_entry(toml_table_t *entry, int index)
{
    LeaderboardEntry *entry_struct = &leaderboard.entries[index];
    toml_datum_t seed = toml_int_in(entry, "seed");
    if (!seed.ok)
    {
        printf("leaderboard entries.%d.seed missing\n", index);
        return;
    }
    entry_struct->seed = seed.u.i;
    toml_datum_t score = toml_int_in(entry, "score");
    if (!score.ok)
    {
        printf("leaderboard entries.%d.score missing\n", index);
        return;
    }
    entry_struct->score = score.u.i;
    toml_datum_t time = toml_int_in(entry, "time");
    if (!time.ok)
    {
        printf("leaderboard entries.%d.time missing\n", index);
        return;
    }
    entry_struct->time = time.u.i;
    toml_datum_t moves = toml_int_in(entry, "moves");
    if (!moves.ok)
    {
        printf("leaderboard entries.%d.moves missing\n", index);
        return;
    }
    entry_struct->moves = moves.u.i;
    toml_datum_t achieved = toml_int_in(entry, "achieved");
    if (!achieved.ok)
    {
        printf("leaderboard entries.%d.achieved missing\n", index);
        return;
    }
    entry_struct->achieved = achieved.u.i;
}

void leaderboard_load_entries(toml_array_t *entries)
{
    int entry_count = min(toml_array_nelem(entries), LEADERBOARD_ENTRY_MAX);

    for (int i = 0; i < entry_count; i++)
    {
        toml_table_t *table = toml_table_at(entries, i);
        if (!table)
        {
            printf("leaderboard entries must be tables\n");
            continue;
        }

        leaderboard_load_entry(table, i);
    }

    leaderboard.entry_count = entry_count;
}

void leaderboard_load()
{
    leaderboard = DEFAULT_LEADERBOARD;

    char *data = LoadFileText("res/leaderboard.toml");

    if (!data)
    {
        printf("failed to read res/leaderboard.toml\n");
        return;
    }

    char errbuf[200];
    toml_table_t *leaderboard_toml = toml_parse(data, errbuf, sizeof(errbuf));
    UnloadFileText(data);

    if (!leaderboard_toml)
    {
        printf("cannot parse - %s\n", errbuf);
        return;
    }

    toml_datum_t high_score = toml_int_in(leaderboard_toml, "high_score");
    if (high_score.ok)
    {
        leaderboard.high_score = high_score.u.i;
    }

    toml_datum_t fastest_time = toml_int_in(leaderboard_toml, "fastest_time");
    if (fastest_time.ok)
    {
        leaderboard.fastest_time = fastest_time.u.i;
    }

    toml_datum_t lowest_moves = toml_int_in(leaderboard_toml, "lowest_moves");
    if (lowest_moves.ok)
    {
        leaderboard.lowest_moves = lowest_moves.u.i;
    }

    toml_array_t *entries = toml_array_in(leaderboard_toml, "entries");
    if (entries)
    {
        leaderboard_load_entries(entries);
    }

    toml_free(leaderboard_toml);
}

void leaderboard_save()
{
    TOML_Writer *writer = toml_writer_new();

    toml_writer_push_integer(writer, "high_score", leaderboard.high_score);
    toml_writer_push_integer(writer, "fastest_time", leaderboard.fastest_time);
    toml_writer_push_integer(writer, "lowest_moves", leaderboard.lowest_moves);

    for (int i = 0; i < leaderboard.entry_count; i++)
    {
        LeaderboardEntry entry = leaderboard.entries[i];
        toml_writer_push_key(writer, "entries", 1);
        toml_writer_push_integer(writer, "seed", entry.seed);
        toml_writer_push_integer(writer, "score", entry.score);
        toml_writer_push_integer(writer, "time", entry.time);
        toml_writer_push_integer(writer, "moves", entry.moves);
        toml_writer_push_integer(writer, "achieved", entry.achieved);
        toml_writer_pop_key(writer);
    }

    toml_writer_save(writer, "res/leaderboard.toml");
    toml_writer_free(writer);
}

// note: this function will save the leaderboard if something changes
void leaderboard_submit(int seed, int score, int game_time, int moves)
{
    int changed = 0;

    if (score > leaderboard.high_score)
    {
        leaderboard.high_score = score;
        changed = 1;
    }

    if (game_time >= 0 && (game_time < leaderboard.fastest_time || leaderboard.fastest_time == -1))
    {
        leaderboard.fastest_time = game_time;
        changed = 1;
    }

    if (moves < leaderboard.lowest_moves || leaderboard.lowest_moves == -1)
    {
        leaderboard.lowest_moves = moves;
        changed = 1;
    }

    int insert_index = 0;

    for (int i = 0; i < leaderboard.entry_count; i++)
    {
        if (score >= leaderboard.entries[i].score)
        {
            continue;
        }

        insert_index = i + 1;
        break;
    }

    if (insert_index < LEADERBOARD_ENTRY_MAX)
    {
        changed = 1;

        int temp_set = 0;
        LeaderboardEntry temp;
        for (int i = insert_index + 1; i < leaderboard.entry_count; i++)
        {
            LeaderboardEntry replaced = leaderboard.entries[i];
            leaderboard.entries[i] = temp_set ? temp : leaderboard.entries[i - 1];
            temp = replaced;
            temp_set = 1;
        }

        int64_t timestamp = (int64_t)time(NULL);
        leaderboard.entries[insert_index] = (LeaderboardEntry){
            .seed = seed,
            .score = score,
            .time = game_time,
            .moves = moves,
            .achieved = timestamp,
        };

        if (leaderboard.entry_count < LEADERBOARD_ENTRY_MAX)
        {
            if (temp_set)
            {
                leaderboard.entries[leaderboard.entry_count] = temp;
            }
            leaderboard.entry_count++;
        }
    }

    if (changed)
    {
        leaderboard_save();
    }
}