#include "game.h"
#include <string.h>
#include <math.h>
#include <stdlib.h> // ← ADD THIS

/* ---------- helpers ---------- */

static float clampf(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

static float len2(float x, float y) { return x * x + y * y; }

static void clamp_speed(Body *b, float max_speed)
{
    float s2 = len2(b->vx, b->vy);
    if (s2 > max_speed * max_speed)
    {
        float s = sqrtf(s2);
        b->vx = b->vx / s * max_speed;
        b->vy = b->vy / s * max_speed;
    }
}

/* Move paddle toward target with acceleration + friction, clamp to half */
static void update_paddle(Body *pad, float tx, float ty, bool is_p1, float dt)
{
    float ax = (tx - pad->x);
    float ay = (ty - pad->y);

    /* deadzone so tiny cursor jitter doesn't jiggle paddle */
    if (len2(ax, ay) < 4.0f)
    {
        pad->vx *= 0.7f;
        pad->vy *= 0.7f;
    }
    else
    {
        float l = sqrtf(len2(ax, ay));
        ax = ax / l * PADDLE_ACCEL;
        ay = ay / l * PADDLE_ACCEL;
        pad->vx += ax * dt;
        pad->vy += ay * dt;
    }

    /* friction */
    pad->vx -= pad->vx * PADDLE_FRICTION * dt;
    pad->vy -= pad->vy * PADDLE_FRICTION * dt;

    clamp_speed(pad, PADDLE_MAX_SPEED);

    pad->x += pad->vx * dt;
    pad->y += pad->vy * dt;

    /* Clamp to own half + walls */
    if (is_p1)
    {
        pad->y = clampf(pad->y, FIELD_H / 2.0f + PADDLE_R + WALL_THICK,
                        FIELD_H - PADDLE_R - WALL_THICK);
    }
    else
    {
        pad->y = clampf(pad->y, PADDLE_R + WALL_THICK,
                        FIELD_H / 2.0f - PADDLE_R - WALL_THICK);
    }
    pad->x = clampf(pad->x, PADDLE_R + WALL_THICK,
                    FIELD_W - PADDLE_R - WALL_THICK);

    /* zero velocity if we hit a wall (so paddle velocity stays honest) */
    if (pad->x <= PADDLE_R + WALL_THICK + 0.1f && pad->vx < 0)
        pad->vx = 0;
    if (pad->x >= FIELD_W - PADDLE_R - WALL_THICK - 0.1f && pad->vx > 0)
        pad->vx = 0;
    if (is_p1 && pad->y >= FIELD_H - PADDLE_R - WALL_THICK - 0.1f && pad->vy > 0)
        pad->vy = 0;
    if (!is_p1 && pad->y <= PADDLE_R + WALL_THICK + 0.1f && pad->vy < 0)
        pad->vy = 0;
}

/* Elastic collision between puck (mass PUCK_MASS) and paddle (mass PADDLE_MASS).
   Paddle treated as infinitely heavy relative to puck for stability,
   but we add paddle velocity contribution for "flick" feel. */
static void collide_paddle(Body *puck, float *spin, const Body *pad)
{
    float dx = puck->x - pad->x;
    float dy = puck->y - pad->y;
    float dist2 = dx * dx + dy * dy;
    float rsum = PUCK_R + PADDLE_R;

    if (dist2 >= rsum * rsum)
        return;
    if (dist2 < 0.0001f)
        dist2 = 0.0001f;

    float dist = sqrtf(dist2);
    float nx = dx / dist;
    float ny = dy / dist;

    /* push puck out so it never overlaps */
    float overlap = rsum - dist + 0.1f;
    puck->x += nx * overlap;
    puck->y += ny * overlap;

    /* relative velocity along normal */
    float rvx = puck->vx - pad->vx;
    float rvy = puck->vy - pad->vy;
    float vn = rvx * nx + rvy * ny;

    if (vn < 0)
    {
        /* Reflection with restitution */
        float j = -(1.0f + COLLISION_RESTITUTION) * vn;
        puck->vx += j * nx;
        puck->vy += j * ny;

        /* spin: tangential component of relative velocity creates spin */
        float tx = -ny, ty = nx;
        float vt = rvx * tx + rvy * ty;
        *spin += vt * 0.0015f;

        /* "punch" from the paddle — makes hard flicks rewarding */
        float paddle_speed = sqrtf(len2(pad->vx, pad->vy));
        if (paddle_speed > 50.0f)
        {
            float boost = 0.35f * paddle_speed;
            puck->vx += nx * boost;
            puck->vy += ny * boost;
        }
    }

    clamp_speed(puck, PUCK_MAX_SPEED);
}

/* Advance puck by small steps to prevent tunneling through thin walls/paddles */
static void integrate_puck(GameState *g, float dt)
{
    float speed = sqrtf(len2(g->puck.vx, g->puck.vy));
    int steps = 1 + (int)(speed * dt / (PUCK_R * 0.5f));
    if (steps > MAX_SUBSTEPS)
        steps = MAX_SUBSTEPS;
    float sdt = dt / steps;

    for (int i = 0; i < steps; i++)
    {
        g->puck.x += g->puck.vx * sdt;
        g->puck.y += g->puck.vy * sdt;

        /* --- paddle collisions --- */
        collide_paddle(&g->puck, &g->puck_spin, &g->p1);
        collide_paddle(&g->puck, &g->puck_spin, &g->p2);

        /* --- goal checks --- */
        float gx_min = (FIELD_W - GOAL_WIDTH) / 2.0f;
        float gx_max = gx_min + GOAL_WIDTH;

        if (g->puck.y - PUCK_R < WALL_THICK)
        {
            if (g->puck.x > gx_min && g->puck.x < gx_max)
            {
                g->score1++;
                g->last_scorer = 1;
                g->goal_flash = 0.6f;
                g->shake = 14.0f;
                game_reset_puck(g, 1);
                return;
            }
            g->puck.y = WALL_THICK + PUCK_R;
            g->puck.vy = -g->puck.vy;
            g->puck.vx *= 0.98f;
        }
        if (g->puck.y + PUCK_R > FIELD_H - WALL_THICK)
        {
            if (g->puck.x > gx_min && g->puck.x < gx_max)
            {
                g->score2++;
                g->last_scorer = 2;
                g->goal_flash = 0.6f;
                g->shake = 14.0f;
                game_reset_puck(g, 2);
                return;
            }
            g->puck.y = FIELD_H - WALL_THICK - PUCK_R;
            g->puck.vy = -g->puck.vy;
            g->puck.vx *= 0.98f;
        }

        /* side walls */
        if (g->puck.x - PUCK_R < WALL_THICK)
        {
            g->puck.x = WALL_THICK + PUCK_R;
            g->puck.vx = -g->puck.vx;
        }
        if (g->puck.x + PUCK_R > FIELD_W - WALL_THICK)
        {
            g->puck.x = FIELD_W - WALL_THICK - PUCK_R;
            g->puck.vx = -g->puck.vx;
        }
    }

    /* Apply spin as slight curve (magnus-ish) */
    float sp = g->puck_spin;
    if (fabsf(sp) > 0.01f)
    {
        float curve = sp * 0.0006f;
        float px = g->puck.vy, py = -g->puck.vx;
        float pl = sqrtf(len2(px, py)) + 0.0001f;
        g->puck.vx += (px / pl) * curve;
        g->puck.vy += (py / pl) * curve;
        g->puck_spin *= 0.985f;
    }
}

/* ---------- public API ---------- */

void game_init(GameState *g, GameMode mode)
{
    memset(g, 0, sizeof(*g));
    g->mode = mode;
    g->running = true;

    g->p1.x = FIELD_W / 2.0f;
    g->p1.y = FIELD_H - 100.0f;
    g->p2.x = FIELD_W / 2.0f;
    g->p2.y = 100.0f;

    game_reset_puck(g, 1);
    g->reset_timer = 3.0f; /* 3-2-1-GO */
}

void game_reset_puck(GameState *g, int toward_player)
{
    g->puck.x = FIELD_W / 2.0f;
    g->puck.y = FIELD_H / 2.0f;
    g->puck.vx = 0;
    g->puck.vy = 0;
    g->puck_spin = 0;
    (void)toward_player; /* ignored — we now relaunch from center slowly */
    g->reset_timer = 3.0f;
}

void game_update(GameState *g, float dt,
                 float p1_tx, float p1_ty,
                 float p2_tx, float p2_ty)
{
    if (dt > 0.05f)
        dt = 0.05f; /* cap huge frame times */

    /* paddles always respond, even during countdown */
    update_paddle(&g->p1, p1_tx, p1_ty, true, dt);
    update_paddle(&g->p2, p2_tx, p2_ty, false, dt);

    /* visual effects tick */
    if (g->goal_flash > 0)
        g->goal_flash -= dt;
    if (g->shake > 0)
        g->shake -= dt * 40.0f;
    if (g->shake < 0)
        g->shake = 0;

    /* countdown */
    if (g->reset_timer > 0)
    {
        g->reset_timer -= dt;
        if (g->reset_timer <= 0)
        {
            /* Launch puck gently toward whoever conceded */
            float dir = (g->last_scorer == 1) ? 1.0f : -1.0f;
            g->puck.vy = 260.0f * dir;
            g->puck.vx = ((rand() % 2) ? 1 : -1) * 80.0f;
        }
        return;
    }

    /* puck friction — proportional to speed */
    float sp = sqrtf(len2(g->puck.vx, g->puck.vy));
    if (sp > 0.01f)
    {
        float drop = PUCK_FRICTION * sp * dt;
        float k = (sp - drop) / sp;
        if (k < 0)
            k = 0;
        g->puck.vx *= k;
        g->puck.vy *= k;
    }
    else
    {
        g->puck.vx = g->puck.vy = 0;
    }
    if (sp < PUCK_MIN_SPEED && sp > 0)
    {
        g->puck.vx = g->puck.vy = 0;
    }

    integrate_puck(g, dt);

    /* safety: if puck is somehow frozen mid-field, nudge it */
    if (g->puck.vx == 0 && g->puck.vy == 0 && g->reset_timer <= 0)
    {
        float dir = (g->puck.y < FIELD_H / 2.0f) ? 1.0f : -1.0f;
        g->puck.vy = 180.0f * dir;
        g->puck.vx = ((rand() % 2) ? 1 : -1) * 60.0f;
    }
}

bool game_check_win(const GameState *g)
{
    return g->score1 >= WIN_SCORE || g->score2 >= WIN_SCORE;
}