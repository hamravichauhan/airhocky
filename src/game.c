#include "game.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

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

static void update_paddle(Body *pad, float tx, float ty, bool is_p1,
                          float dt, float radius)
{
    float ax = (tx - pad->x);
    float ay = (ty - pad->y);

    if (len2(ax, ay) < 4.0f)
    {
        pad->vx *= 0.7f;
        pad->vy *= 0.7f;
    }
    else
    {
        float l = sqrtf(len2(ax, ay));
        ax = ax / l * RT_PADDLE_ACCEL;
        ay = ay / l * RT_PADDLE_ACCEL;
        pad->vx += ax * dt;
        pad->vy += ay * dt;
    }

    pad->vx -= pad->vx * PADDLE_FRICTION * dt;
    pad->vy -= pad->vy * PADDLE_FRICTION * dt;

    clamp_speed(pad, RT_PADDLE_MAX);

    pad->x += pad->vx * dt;
    pad->y += pad->vy * dt;

    if (is_p1)
    {
        pad->y = clampf(pad->y, FIELD_H / 2.0f + radius + WALL_THICK,
                        FIELD_H - radius - WALL_THICK);
    }
    else
    {
        pad->y = clampf(pad->y, radius + WALL_THICK,
                        FIELD_H / 2.0f - radius - WALL_THICK);
    }
    pad->x = clampf(pad->x, radius + WALL_THICK,
                    FIELD_W - radius - WALL_THICK);

    if (pad->x <= radius + WALL_THICK + 0.1f && pad->vx < 0)
        pad->vx = 0;
    if (pad->x >= FIELD_W - radius - WALL_THICK - 0.1f && pad->vx > 0)
        pad->vx = 0;
    if (is_p1 && pad->y >= FIELD_H - radius - WALL_THICK - 0.1f && pad->vy > 0)
        pad->vy = 0;
    if (!is_p1 && pad->y <= radius + WALL_THICK + 0.1f && pad->vy < 0)
        pad->vy = 0;
}

static void collide_paddle(GameState *g, Body *puck, float *spin,
                           const Body *pad, float pad_radius)
{
    float dx = puck->x - pad->x;
    float dy = puck->y - pad->y;
    float dist2 = dx * dx + dy * dy;
    float rsum = PUCK_R + pad_radius;

    if (dist2 >= rsum * rsum)
        return;
    if (dist2 < 0.0001f)
        dist2 = 0.0001f;

    float dist = sqrtf(dist2);
    float nx = dx / dist;
    float ny = dy / dist;

    float overlap = rsum - dist + 0.1f;
    puck->x += nx * overlap;
    puck->y += ny * overlap;

    float rvx = puck->vx - pad->vx;
    float rvy = puck->vy - pad->vy;
    float vn = rvx * nx + rvy * ny;

    if (vn < 0)
    {
        float j = -(1.0f + COLLISION_RESTITUTION) * vn;
        puck->vx += j * nx;
        puck->vy += j * ny;

        float tx = -ny, ty = nx;
        float vt = rvx * tx + rvy * ty;
        *spin += vt * 0.0015f;

        if (pad == &g->p1 && g->p1_spin_bias != 0.0f)
            *spin += g->p1_spin_bias * 12.0f;
        if (pad == &g->p2 && g->p2_spin_bias != 0.0f)
            *spin += g->p2_spin_bias * 12.0f;

        float paddle_speed = sqrtf(len2(pad->vx, pad->vy));
        if (paddle_speed > 50.0f)
        {
            float boost = 0.35f * paddle_speed;
            puck->vx += nx * boost;
            puck->vy += ny * boost;
        }

        g->ev_hit = true;
        float imp = fabsf(vn) / 500.0f + paddle_speed / 900.0f;
        if (imp > 1.0f)
            imp = 1.0f;
        if (imp < 0.2f)
            imp = 0.2f;
        g->ev_hit_intensity = imp;

        g->ev_hit_pos_valid = true;
        g->ev_hit_pos_x = puck->x;
        g->ev_hit_pos_y = puck->y;
        g->ev_hit_dir_x = nx;
        g->ev_hit_dir_y = ny;
    }

    clamp_speed(puck, RT_PUCK_MAX_SPEED);
}

static void integrate_puck(GameState *g, float dt, float r1, float r2)
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

        collide_paddle(g, &g->puck, &g->puck_spin, &g->p1, r1);
        collide_paddle(g, &g->puck, &g->puck_spin, &g->p2, r2);

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
                g->ev_goal = true;
                game_reset_puck(g, 1);
                return;
            }
            g->puck.y = WALL_THICK + PUCK_R;
            g->puck.vy = -g->puck.vy;
            g->puck.vx *= 0.98f;
            g->ev_wall = true;
            g->ev_wall_intensity = 0.55f;
            g->ev_wall_pos_valid = true;
            g->ev_wall_pos_x = g->puck.x;
            g->ev_wall_pos_y = g->puck.y;
        }
        if (g->puck.y + PUCK_R > FIELD_H - WALL_THICK)
        {
            if (g->puck.x > gx_min && g->puck.x < gx_max)
            {
                g->score2++;
                g->last_scorer = 2;
                g->goal_flash = 0.6f;
                g->shake = 14.0f;
                g->ev_goal = true;
                game_reset_puck(g, 2);
                return;
            }
            g->puck.y = FIELD_H - WALL_THICK - PUCK_R;
            g->puck.vy = -g->puck.vy;
            g->puck.vx *= 0.98f;
            g->ev_wall = true;
            g->ev_wall_intensity = 0.55f;
            g->ev_wall_pos_valid = true;
            g->ev_wall_pos_x = g->puck.x;
            g->ev_wall_pos_y = g->puck.y;
        }

        if (g->puck.x - PUCK_R < WALL_THICK)
        {
            g->puck.x = WALL_THICK + PUCK_R;
            g->puck.vx = -g->puck.vx;
            g->ev_wall = true;
            g->ev_wall_intensity = 0.5f;
            g->ev_wall_pos_valid = true;
            g->ev_wall_pos_x = g->puck.x;
            g->ev_wall_pos_y = g->puck.y;
        }
        if (g->puck.x + PUCK_R > FIELD_W - WALL_THICK)
        {
            g->puck.x = FIELD_W - WALL_THICK - PUCK_R;
            g->puck.vx = -g->puck.vx;
            g->ev_wall = true;
            g->ev_wall_intensity = 0.5f;
            g->ev_wall_pos_valid = true;
            g->ev_wall_pos_x = g->puck.x;
            g->ev_wall_pos_y = g->puck.y;
        }
    }

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

    g->p1_radius_mult = 1.0f;
    g->p2_radius_mult = 1.0f;
    g->puck_speed_mult = 1.0f;
    g->p1_frozen = false;
    g->p2_frozen = false;

    game_reset_puck(g, 1);
    g->reset_timer = 3.0f;
}

void game_reset_puck(GameState *g, int toward_player)
{
    g->puck.x = FIELD_W / 2.0f;
    g->puck.y = FIELD_H / 2.0f;
    g->puck.vx = 0;
    g->puck.vy = 0;
    g->puck_spin = 0;
    (void)toward_player;
    g->reset_timer = 3.0f;
}

void game_update(GameState *g, float dt,
                 float p1_tx, float p1_ty,
                 float p2_tx, float p2_ty)
{
    if (dt > 0.05f)
        dt = 0.05f;

    g->ev_hit = g->ev_wall = g->ev_goal = false;
    g->ev_go = false;
    g->ev_count_tick = -1;
    g->ev_hit_pos_valid = false;
    g->ev_wall_pos_valid = false;

    float p1_radius = PADDLE_R * g->p1_radius_mult;
    float p2_radius = PADDLE_R * g->p2_radius_mult;

    if (g->p1_frozen)
    {
        g->p1.vx = g->p1.vy = 0;
    }
    else
    {
        update_paddle(&g->p1, p1_tx, p1_ty, true, dt, p1_radius);
    }

    if (g->p2_frozen)
    {
        g->p2.vx = g->p2.vy = 0;
    }
    else
    {
        update_paddle(&g->p2, p2_tx, p2_ty, false, dt, p2_radius);
    }

    if (g->goal_flash > 0)
        g->goal_flash -= dt;
    if (g->shake > 0)
        g->shake -= dt * 40.0f;
    if (g->shake < 0)
        g->shake = 0;

    if (g->reset_timer > 0)
    {
        int prev_sec = (int)ceilf(g->reset_timer + dt);
        g->reset_timer -= dt;
        int curr_sec = (int)ceilf(g->reset_timer);

        if (prev_sec != curr_sec)
        {
            if (curr_sec >= 1 && curr_sec <= 3)
                g->ev_count_tick = curr_sec;
            else if (curr_sec <= 0)
                g->ev_go = true;
        }

        if (g->reset_timer <= 0)
        {
            float dir = (g->last_scorer == 1) ? 1.0f : -1.0f;
            g->puck.vy = 260.0f * dir;
            g->puck.vx = ((rand() % 2) ? 1 : -1) * 80.0f;
        }
        return;
    }

    float sp = sqrtf(len2(g->puck.vx, g->puck.vy));
    if (sp > 0.01f)
    {
        float drop = RT_PUCK_FRICTION * sp * dt;
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

    integrate_puck(g, dt, p1_radius, p2_radius);

    if (g->puck_speed_mult < 1.0f)
    {
        float s2 = sqrtf(len2(g->puck.vx, g->puck.vy));
        float cap = RT_PUCK_MAX_SPEED * g->puck_speed_mult;
        if (s2 > cap && s2 > 0.01f)
        {
            float k = cap / s2;
            g->puck.vx *= k;
            g->puck.vy *= k;
        }
    }

    if (g->puck.vx == 0 && g->puck.vy == 0 && g->reset_timer <= 0)
    {
        float dir = (g->puck.y < FIELD_H / 2.0f) ? 1.0f : -1.0f;
        g->puck.vy = 180.0f * dir;
        g->puck.vx = ((rand() % 2) ? 1 : -1) * 60.0f;
    }
}

bool game_check_win(const GameState *g)
{
    return g->score1 >= RT_WIN_SCORE || g->score2 >= RT_WIN_SCORE;
}

void game_apply_spin_bias(GameState *g, int player, float bias)
{
    if (player == 1)
        g->p1_spin_bias = bias;
    else
        g->p2_spin_bias = bias;
}