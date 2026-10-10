#include "coach.h"
#include "features.h"
#include <string.h>
#include <stdio.h>

#if FEATURE_AI_COACH

void coach_clear(CoachReport *rep)
{
    memset(rep, 0, sizeof(*rep));
}

void coach_analyze(CoachReport *rep, const MatchStats *ms, int you_won)
{
    coach_clear(rep);

    int total_hits = ms->p1_hits + ms->p2_hits;
    if (total_hits == 0)
    {
        strcpy(rep->tips[rep->count++], "Barely any contact — try moving closer to the puck.");
        rep->valid = true;
        return;
    }

    float p1_accuracy = (ms->p1_hits > 0)
                            ? (float)ms->goals_p1 / (float)ms->p1_hits * 100.0f
                            : 0;
    /* --- Analysis rules --- */

    /* 1. Rally length */
    if (ms->best_rally < 5)
    {
        strcpy(rep->tips[rep->count++],
               "Your rallies were short. Aim for controlled shots, not just fast ones.");
    }
    else if (ms->best_rally >= 20)
    {
        strcpy(rep->tips[rep->count++],
               "Excellent rally control! You kept the puck in play for a long time.");
    }

    /* 2. Accuracy */
    if (p1_accuracy < 10.0f && ms->p1_hits > 5)
    {
        strcpy(rep->tips[rep->count++],
               "Low shot accuracy — you're hitting the puck often but rarely scoring.");
    }
    else if (p1_accuracy > 35.0f)
    {
        strcpy(rep->tips[rep->count++],
               "Great shot accuracy! Most of your hits became goals.");
    }

    /* 3. Puck speed — was the player too passive? */
    if (ms->top_puck_speed < 400.0f)
    {
        strcpy(rep->tips[rep->count++],
               "The puck never got fast. Use flick shots — move paddle fast into the puck.");
    }
    else if (ms->top_puck_speed > 1000.0f)
    {
        strcpy(rep->tips[rep->count++],
               "You hit with power — opponent had trouble keeping up.");
    }

    /* 4. Hits vs conceded */
    if (ms->goals_conceded_p1 >= 5)
    {
        strcpy(rep->tips[rep->count++],
               "Too many goals conceded. Try sitting closer to your goal, not midfield.");
    }

    /* 5. Symmetric play */
    if (ms->p1_hits > 0 && ms->p2_hits > 0)
    {
        float ratio = (float)ms->p1_hits / (float)ms->p2_hits;
        if (ratio < 0.5f)
        {
            strcpy(rep->tips[rep->count++],
                   "You barely touched the puck. Stay aggressive — meet the puck early.");
        }
        else if (ratio > 2.0f)
        {
            strcpy(rep->tips[rep->count++],
                   "You were dominant. Try a harder AI to keep it challenging.");
        }
    }

    /* 6. Win/loss comment (always) */
    if (you_won)
    {
        strcpy(rep->tips[rep->count++],
               "Solid victory — keep the pressure on early in rallies.");
    }
    else
    {
        strcpy(rep->tips[rep->count++],
               "Tough loss. Focus on defense first, then counter-attacks.");
    }

    rep->valid = true;
}

#else
void coach_clear(CoachReport *rep) { memset(rep, 0, sizeof(*rep)); }
void coach_analyze(CoachReport *rep, const MatchStats *ms, int you_won)
{
    (void)rep;
    (void)ms;
    (void)you_won;
}
#endif