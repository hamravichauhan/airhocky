#include "trajectory.h"
#include "features.h"
#include <math.h>

#if FEATURE_TRAJECTORY

void trajectory_draw(const GameState *g, int offset_y, bool enabled)
{
    if (!enabled)
        return;

    /* Simulate puck forward using mirror-reflection math for x, simple for y */
    float px = g->puck.x;
    float py = g->puck.y;
    float vx = g->puck.vx;
    float vy = g->puck.vy;

    /* Kill speed if puck nearly stopped */
    float sp = sqrtf(vx * vx + vy * vy);
    if (sp < 30.0f)
        return;

    /* Trace forward, drawing dots */
    const float DT = 0.05f;
    const int MAX_STEPS = 60; /* 3 seconds of lookahead max */

    float left = WALL_THICK + PUCK_R;
    float right = FIELD_W - WALL_THICK - PUCK_R;
    float top = WALL_THICK + PUCK_R;
    float bot = FIELD_H - WALL_THICK - PUCK_R;

    /* Goal gap: no bounce if within goal range on top/bottom */
    float gx_min = (FIELD_W - GOAL_WIDTH) / 2.0f;
    float gx_max = gx_min + GOAL_WIDTH;

    for (int i = 0; i < MAX_STEPS; i++)
    {
        px += vx * DT;
        py += vy * DT;

        /* side walls */
        if (px < left)
        {
            px = left;
            vx = -vx;
        }
        if (px > right)
        {
            px = right;
            vx = -vx;
        }

        /* top/bottom walls — no bounce inside goal gap */
        bool in_goal_x = (px > gx_min && px < gx_max);
        if (py < top)
        {
            if (in_goal_x)
                break; /* puck would enter goal — stop tracing */
            py = top;
            vy = -vy;
        }
        if (py > bot)
        {
            if (in_goal_x)
                break;
            py = bot;
            vy = -vy;
        }

        /* fade out dot as we go further */
        float t = 1.0f - (float)i / MAX_STEPS;
        unsigned char a = (unsigned char)(120 * t);

        DrawCircle((int)px, (int)py + offset_y, 3,
                   (Color){255, 220, 100, a});
    }
}

#else
void trajectory_draw(const GameState *g, int offset_y, bool enabled)
{
    (void)g;
    (void)offset_y;
    (void)enabled;
}
#endif