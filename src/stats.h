#ifndef STATS_H
#define STATS_H

#include "game.h"
#include <stdbool.h>

typedef enum
{
    HYPE_NONE = 0,
    HYPE_NICE,
    HYPE_GREAT,
    HYPE_INSANE,
    HYPE_LEGENDARY
} HypeLevel;

typedef struct
{
    int current_rally;
    int best_rally;

    int p1_hits;
    int p2_hits;

    HypeLevel hype;
    float hype_timer;
    float hype_pulse;

    int p1_shots;
    int p2_shots;
    int goals_p1;
    int goals_p2;
    int goals_conceded_p1;
    int goals_conceded_p2;
    float top_puck_speed;
    float match_time;

    /* power-up tally this match */
    int p1_powerups;
    int p2_powerups;
} MatchStats;

/* Persistent records — saved to disk between runs */
typedef struct
{
    int best_rally_ever;
    int most_goals_p1_ever;
    int most_goals_p2_ever;
    float fastest_puck_ever;
    int matches_played;
} Records;

/* --- API --- */
void stats_init(MatchStats *s);
void stats_update(MatchStats *s, const GameState *g, float dt);
void stats_on_hit(MatchStats *s, int player);
void stats_on_goal(MatchStats *s, int scoring_player);

const char *stats_hype_text(const MatchStats *s);

/* --- Records persistence --- */
void records_load(Records *r);
void records_save(const Records *r);
bool records_check_and_update(MatchStats *ms, Records *r);

#endif