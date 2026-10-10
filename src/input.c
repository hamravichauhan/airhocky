#include "input.h"
#include "raylib.h"
#include <math.h> // ← ADD THIS

/* Mouse-based + keyboard hybrid: paddle chases the mouse in your half if
   mouse is on your side, otherwise uses keyboard. Both feel great. */

void input_get_local_target(const GameState *g, int paddle_id,
                            float *out_x, float *out_y)
{
    /* start from current paddle position */
    const Body *pad = (paddle_id == 1) ? &g->p1 : &g->p2;
    float x = pad->x, y = pad->y;

    if (g->mode == MODE_LOCAL)
    {
        if (paddle_id == 1)
        {
            /* P1: WASD */
            float sp = 700.0f * GetFrameTime();
            if (IsKeyDown(KEY_W))
                y -= sp;
            if (IsKeyDown(KEY_S))
                y += sp;
            if (IsKeyDown(KEY_A))
                x -= sp;
            if (IsKeyDown(KEY_D))
                x += sp;
        }
        else
        {
            /* P2: arrows */
            float sp = 700.0f * GetFrameTime();
            if (IsKeyDown(KEY_UP))
                y -= sp;
            if (IsKeyDown(KEY_DOWN))
                y += sp;
            if (IsKeyDown(KEY_LEFT))
                x -= sp;
            if (IsKeyDown(KEY_RIGHT))
                x += sp;
        }
    }
    else
    {
        /* Network modes: this client controls P1 only */
        float sp = 700.0f * GetFrameTime();
        if (IsKeyDown(KEY_W))
            y -= sp;
        if (IsKeyDown(KEY_S))
            y += sp;
        if (IsKeyDown(KEY_A))
            x -= sp;
        if (IsKeyDown(KEY_D))
            x += sp;

        /* Mouse feel: if mouse is in own half, use mouse position
           offset by the field's top margin (100px). */
        Vector2 m = GetMousePosition();
        float my = m.y - 100;
        bool mouse_in_own_half = (paddle_id == 1)
                                     ? (my > FIELD_H / 2.0f)
                                     : (my < FIELD_H / 2.0f);
        if (mouse_in_own_half && (fabsf(m.x - x) > 3 || fabsf(my - y) > 3))
        {
            x = m.x;
            y = my;
        }
    }

    *out_x = x;
    *out_y = y;
}