#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include "game.h"
#include <stdbool.h>

typedef enum
{
    TOUR_BO3, /* best of 3 — first to 2 wins */
    TOUR_BO5  /* best of 5 — first to 3 wins */
} TournamentFormat;

typedef struct
{
    TournamentFormat format;
    int wins_p1;
    int wins_p2;
    int wins_needed;   /* 2 for BO3, 3 for BO5 */
    int current_match; /* 1-based match number */
    int match_goal;    /* goals to win each match — 7 by default */

    /* Per-match win tracking */
    int last_winner;

    bool active;       /* is a tournament in progress? */
    bool series_over;  /* completed? */
    int series_winner; /* 1, 2, or 0 */
} Tournament;

void tournament_init(Tournament *t, TournamentFormat fmt);
void tournament_record_match(Tournament *t, int winner);
bool tournament_is_over(const Tournament *t);

#endif