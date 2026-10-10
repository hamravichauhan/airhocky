#ifndef STATS_H
#define STATS_H

#include "game.h"
#include <stdbool.h>

/* Hype levels triggered at combo thresholds */
typedef enum
{
    HYPE_NONE = 0,
    HYPE_NICE,     /* 5 hits */
    HYPE_GREAT,    /* 10 hits */
    HYPE_INSANE,   /* 20 hits */
    HYPE_LEGENDARY /* 30 hits */
} HypeLevel;

typedef struct
{
    int current_rally; /* consecutive hits without a goal */
    int best_rally;    /* longest rally this match */

    /* per-player breakdown */
    int p1_hits;
    int p2_hits;

    /* Hype event — set when threshold crossed, cleared by main after playing */
    HypeLevel hype;
    float hype_timer; /* how long to display hype text */
    float hype_pulse; /* animation phase */

    /* Per-match stats (used by Feature #2 later) */
    int p1_shots; /* total hits by P1 */
    int p2_shots;
    int goals_p1;
    int goals_p2;
    int goals_conceded_p1;
    int goals_conceded_p2;
    float top_puck_speed;
    float match_time; /* seconds elapsed in current match */
} MatchStats;

/* --- API --- */
void stats_init(MatchStats *s);
void stats_update(MatchStats *s, const GameState *g, float dt);
void stats_on_hit(MatchStats *s, int player);
void stats_on_goal(MatchStats *s, int scoring_player);

/* Read current hype text (or NULL if none) */
const char *stats_hype_text(const MatchStats *s);

#endif