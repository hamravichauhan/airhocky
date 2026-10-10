#ifndef COMMENTARY_H
#define COMMENTARY_H

#include "raylib.h"
#include <stdbool.h>

typedef enum
{
    CMT_NONE = 0,
    CMT_GREAT_SAVE,
    CMT_POWER_SHOT,
    CMT_GOAL_P1,
    CMT_GOAL_P2,
    CMT_RALLY_10,
    CMT_RALLY_20,
    CMT_INSANE
} CommentaryLine;

typedef struct
{
    CommentaryLine current;
    float timer;    /* seconds remaining */
    float duration; /* total duration */
    bool shown;     /* has the voice beep fired yet? */
} Commentary;

void commentary_init(Commentary *c);
void commentary_trigger(Commentary *c, CommentaryLine line);
void commentary_update(Commentary *c, float dt);
void commentary_draw(const Commentary *c, int offset_y);

const char *commentary_text(CommentaryLine line);
bool commentary_voice_played(const Commentary *c);

#endif