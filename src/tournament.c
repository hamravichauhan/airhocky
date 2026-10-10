#include "tournament.h"
#include <string.h>

void tournament_init(Tournament *t, TournamentFormat fmt)
{
    memset(t, 0, sizeof(*t));
    t->format = fmt;
    t->wins_needed = (fmt == TOUR_BO3) ? 2 : 3;
    t->current_match = 1;
    t->match_goal = WIN_SCORE;
    t->active = true;
    t->series_over = false;
}

void tournament_record_match(Tournament *t, int winner)
{
    if (winner == 1)
        t->wins_p1++;
    else
        t->wins_p2++;
    t->last_winner = winner;
    t->current_match++;

    if (t->wins_p1 >= t->wins_needed || t->wins_p2 >= t->wins_needed)
    {
        t->series_over = true;
        t->series_winner = (t->wins_p1 > t->wins_p2) ? 1 : 2;
    }
}

bool tournament_is_over(const Tournament *t)
{
    return t->series_over;
}