#include "commentary.h"
#include "game.h" // ← ADD THIS (for FIELD_W)
#include "features.h"
#include <string.h>
#include <stdio.h>

#if FEATURE_COMMENTARY

static const char *texts[] = {
    [CMT_NONE] = "",
    [CMT_GREAT_SAVE] = "GREAT SAVE!",
    [CMT_POWER_SHOT] = "WHAT A SHOT!",
    [CMT_GOAL_P1] = "P1 SCORES!",
    [CMT_GOAL_P2] = "P2 SCORES!",
    [CMT_RALLY_10] = "INCREDIBLE RALLY!",
    [CMT_RALLY_20] = "THIS IS INSANE!",
    [CMT_INSANE] = "UNBELIEVABLE!",
};

void commentary_init(Commentary *c)
{
    memset(c, 0, sizeof(*c));
    c->duration = 1.6f;
}

void commentary_trigger(Commentary *c, CommentaryLine line)
{
    if (line == CMT_NONE)
        return;
    c->current = line;
    c->timer = c->duration;
    c->shown = false;
}

void commentary_update(Commentary *c, float dt)
{
    if (c->timer > 0)
    {
        c->timer -= dt;
        if (c->timer < 0)
            c->timer = 0;
    }
}

void commentary_draw(const Commentary *c, int offset_y)
{
    if (c->timer <= 0)
        return;

    const char *txt = commentary_text(c->current);
    if (!txt || !txt[0])
        return;

    /* fade in first 0.2s, fade out last 0.4s */
    float elapsed = c->duration - c->timer;
    float alpha = 1.0f;
    if (elapsed < 0.2f)
        alpha = elapsed / 0.2f;
    if (c->timer < 0.4f)
        alpha = c->timer / 0.4f;
    if (alpha > 1.0f)
        alpha = 1.0f;
    if (alpha < 0.0f)
        alpha = 0.0f;

    unsigned char a = (unsigned char)(255 * alpha);

    int fs = 30;
    int tw = MeasureText(txt, fs);

    /* dark backdrop */
    int bx = FIELD_W / 2 - tw / 2 - 20;
    int by = offset_y + 10;
    DrawRectangle(bx, by, tw + 40, fs + 16,
                  (Color){0, 0, 0, (unsigned char)(120 * alpha)});

    Color col = (Color){255, 220, 100, a};
    switch (c->current)
    {
    case CMT_GREAT_SAVE:
        col = (Color){120, 220, 255, a};
        break;
    case CMT_POWER_SHOT:
        col = (Color){255, 140, 80, a};
        break;
    case CMT_GOAL_P1:
        col = (Color){100, 180, 255, a};
        break;
    case CMT_GOAL_P2:
        col = (Color){255, 100, 100, a};
        break;
    case CMT_RALLY_10:
        col = (Color){180, 255, 100, a};
        break;
    case CMT_RALLY_20:
        col = (Color){255, 180, 80, a};
        break;
    case CMT_INSANE:
        col = (Color){255, 100, 200, a};
        break;
    default:
        break;
    }

    DrawText(txt, FIELD_W / 2 - tw / 2, by + 8, fs, col);
}

const char *commentary_text(CommentaryLine line)
{
    if (line < 0 || line > CMT_INSANE)
        return "";
    return texts[line];
}

bool commentary_voice_played(const Commentary *c)
{
    return c->shown;
}

#else /* disabled */
void commentary_init(Commentary *c) { memset(c, 0, sizeof(*c)); }
void commentary_trigger(Commentary *c, CommentaryLine line)
{
    (void)c;
    (void)line;
}
void commentary_update(Commentary *c, float dt)
{
    (void)c;
    (void)dt;
}
void commentary_draw(const Commentary *c, int offset_y)
{
    (void)c;
    (void)offset_y;
}
const char *commentary_text(CommentaryLine line)
{
    (void)line;
    return "";
}
bool commentary_voice_played(const Commentary *c)
{
    (void)c;
    return true;
}
#endif