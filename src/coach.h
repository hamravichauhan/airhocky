#ifndef COACH_H
#define COACH_H

#include "stats.h"
#include <stdbool.h>

#define COACH_MAX_TIPS 6

typedef struct
{
    char tips[COACH_MAX_TIPS][128];
    int count;
    bool valid; /* set true when tips were generated */
} CoachReport;

void coach_analyze(CoachReport *rep, const MatchStats *ms, int you_won);
void coach_clear(CoachReport *rep);

#endif