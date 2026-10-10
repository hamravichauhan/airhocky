#include "stats.h"
#include "features.h"
#include <string.h>
#include <math.h>

#if FEATURE_COMBO_COUNTER

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

    /* Trigger hype on exact thresholds */
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
}

void stats_on_goal(MatchStats *s, int scoring_player)
{
    /* Reset rally */
    s->current_rally = 0;

    /* Score counters */
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

    /* Track top puck speed */
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

#else /* FEATURE_COMBO_COUNTER disabled — stubs */

void stats_init(MatchStats *s) { memset(s, 0, sizeof(*s)); }
void stats_on_hit(MatchStats *s, int p)
{
    (void)s;
    (void)p;
}
void stats_on_goal(MatchStats *s, int p)
{
    (void)s;
    (void)p;
}
void stats_update(MatchStats *s, const GameState *g, float dt)
{
    (void)s;
    (void)g;
    (void)dt;
}
const char *stats_hype_text(const MatchStats *s)
{
    (void)s;
    return NULL;
}

#endif