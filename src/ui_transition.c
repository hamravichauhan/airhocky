#include "ui_transition.h"
#include <string.h>

void ui_trans_init(UIScreenTransition *t)
{
    memset(t, 0, sizeof(*t));
    t->dir = -1; /* start faded out → fade in */
    t->fade = 1.0f;
}

void ui_trans_update(UIScreenTransition *t, float dt)
{
    const float SPEED = 3.0f;
    if (t->dir > 0)
    {
        t->fade += SPEED * dt;
        if (t->fade >= 1.0f)
        {
            t->fade = 1.0f;
            /* swap screen now */
            t->pending_switch = true;
            t->dir = -1;
        }
    }
    else if (t->dir < 0)
    {
        t->fade -= SPEED * dt;
        if (t->fade <= 0.0f)
        {
            t->fade = 0.0f;
            t->dir = 0;
        }
    }
}

bool ui_trans_begin(UIScreenTransition *t)
{
    if (t->dir == 0)
    {
        t->dir = 1;
        return false;
    }
    return false;
}

void ui_trans_draw(const UIScreenTransition *t)
{
    if (t->fade <= 0.01f)
        return;
    unsigned char a = (unsigned char)(255 * t->fade);
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                  (Color){0, 0, 0, a});
}