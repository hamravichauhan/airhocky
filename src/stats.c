#include "stats.h"
#include "features.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

/* ============================================================
   Match stats
   ============================================================ */

void stats_init(MatchStats *s)
{
    memset(s, 0, sizeof(*s));
    s->hype = HYPE_NONE;
    s->hype_timer = 0;
    s->hype_pulse = 0;
}

void stats_on_hit(MatchStats *s, int player)
{
    s->current_rally++;
    if (s->current_rally > s->best_rally)
        s->best_rally = s->current_rally;

    if (player == 1)
        s->p1_hits++;
    else
        s->p2_hits++;

#if FEATURE_COMBO_COUNTER
    HypeLevel new_hype = HYPE_NONE;
    if (s->current_rally == 5)
        new_hype = HYPE_NICE;
    if (s->current_rally == 10)
        new_hype = HYPE_GREAT;
    if (s->current_rally == 20)
        new_hype = HYPE_INSANE;
    if (s->current_rally == 30)
        new_hype = HYPE_LEGENDARY;

    if (new_hype != HYPE_NONE)
    {
        s->hype = new_hype;
        s->hype_timer = 1.8f;
        s->hype_pulse = 0;
    }
#endif
}

void stats_on_goal(MatchStats *s, int scoring_player)
{
    s->current_rally = 0;

    if (scoring_player == 1)
    {
        s->goals_p1++;
        s->goals_conceded_p2++;
    }
    else
    {
        s->goals_p2++;
        s->goals_conceded_p1++;
    }
}

void stats_update(MatchStats *s, const GameState *g, float dt)
{
    s->match_time += dt;

    if (s->hype_timer > 0)
    {
        s->hype_timer -= dt;
        s->hype_pulse += dt;
        if (s->hype_timer <= 0)
        {
            s->hype = HYPE_NONE;
            s->hype_timer = 0;
        }
    }

    float sp = sqrtf(g->puck.vx * g->puck.vx + g->puck.vy * g->puck.vy);
    if (sp > s->top_puck_speed)
        s->top_puck_speed = sp;
}

const char *stats_hype_text(const MatchStats *s)
{
    switch (s->hype)
    {
    case HYPE_NICE:
        return "NICE!";
    case HYPE_GREAT:
        return "GREAT!";
    case HYPE_INSANE:
        return "INSANE!";
    case HYPE_LEGENDARY:
        return "LEGENDARY!";
    default:
        return NULL;
    }
}

/* ============================================================
   Persistent records (mini "database")
   ============================================================ */

void records_load(Records *r)
{
    r->best_rally_ever = 0;
    r->most_goals_p1_ever = 0;
    r->most_goals_p2_ever = 0;
    r->fastest_puck_ever = 0;
    r->matches_played = 0;

    FILE *f = fopen("airhockey.dat", "r");
    if (!f)
        return;

    if (fscanf(f, "%d\n%d\n%d\n%f\n%d",
               &r->best_rally_ever,
               &r->most_goals_p1_ever,
               &r->most_goals_p2_ever,
               &r->fastest_puck_ever,
               &r->matches_played) != 5)
    {
        /* corrupt file — keep defaults */
    }
    fclose(f);
}

void records_save(const Records *r)
{
    FILE *f = fopen("airhockey.dat", "w");
    if (!f)
        return;
    fprintf(f, "%d\n%d\n%d\n%.2f\n%d\n",
            r->best_rally_ever,
            r->most_goals_p1_ever,
            r->most_goals_p2_ever,
            r->fastest_puck_ever,
            r->matches_played);
    fclose(f);
}

bool records_check_and_update(MatchStats *ms, Records *r)
{
    bool new_record = false;

    if (ms->best_rally > r->best_rally_ever)
    {
        r->best_rally_ever = ms->best_rally;
        new_record = true;
    }
    if (ms->goals_p1 > r->most_goals_p1_ever)
    {
        r->most_goals_p1_ever = ms->goals_p1;
        new_record = true;
    }
    if (ms->goals_p2 > r->most_goals_p2_ever)
    {
        r->most_goals_p2_ever = ms->goals_p2;
        new_record = true;
    }
    if (ms->top_puck_speed > r->fastest_puck_ever)
    {
        r->fastest_puck_ever = ms->top_puck_speed;
        new_record = true;
    }

    r->matches_played++;
    records_save(r);
    return new_record;
}