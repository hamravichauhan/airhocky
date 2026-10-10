#include "input.h"
#include "raylib.h"

void input_get_local_target(const GameState *g, int paddle_id,
                            float *out_x, float *out_y)
{
    const Body *pad = (paddle_id == 1) ? &g->p1 : &g->p2;
    float x = pad->x, y = pad->y;

    /* Solo mode: P2 is AI. If asked, return current pos (no movement). */
    if (g->mode == MODE_SOLO && paddle_id == 2)
    {
        *out_x = x;
        *out_y = y;
        return;
    }

    float sp = 700.0f * GetFrameTime();

    if (g->mode == MODE_LOCAL)
    {
        if (paddle_id == 1)
        {
            /* P1 — WASD */
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
            /* P2 — Arrow keys */
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
        /* Solo, Host, Client — this machine always controls P1 with WASD */
        if (IsKeyDown(KEY_W))
            y -= sp;
        if (IsKeyDown(KEY_S))
            y += sp;
        if (IsKeyDown(KEY_A))
            x -= sp;
        if (IsKeyDown(KEY_D))
            x += sp;
    }

    *out_x = x;
    *out_y = y;
}