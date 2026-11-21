#ifndef UI_TRANSITION_H
#define UI_TRANSITION_H

#include "raylib.h"
#include <stdbool.h>

typedef struct
{
    float fade;          /* 0..1 */
    int dir;             /* +1 fading out, -1 fading in */
    bool pending_switch; /* true while waiting to swap screens */
} UIScreenTransition;

void ui_trans_init(UIScreenTransition *t);
void ui_trans_update(UIScreenTransition *t, float dt);

/* Call this when you want to switch screens.
   Returns true the frame the swap should happen. */
bool ui_trans_begin(UIScreenTransition *t);

/* Draw black overlay on top of everything */
void ui_trans_draw(const UIScreenTransition *t);

#endif