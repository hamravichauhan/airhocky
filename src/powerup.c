#include "powerup.h"
#include <math.h>
#include <stdlib.h>

/* forward decl */
static void apply_powerup(PowerUpState *ps, PowerUpType type, int player);

/* ---------- helpers ---------- */

static float randf(void)
{
    return (float)rand() / (float)RAND_MAX;
}

static float rand_range(float lo, float hi)
{
    return lo + randf() * (hi - lo);
}

static PowerUpType rand_type(void)
{
    return (PowerUpType)(PU_GROW + (rand() % 3));
}

/* Spawn a mirrored pair. P1 drop goes in bottom half, P2 in top half. */
static void spawn_pair(PowerUpState *ps)
{
    PowerUpType t = rand_type();

    /* Random X in the middle 60% of the field */
    float x = rand_range(FIELD_W * 0.25f, FIELD_W * 0.75f);

    /* Random Y in the middle of each half */
    float y_bottom = rand_range(FIELD_H * 0.62f, FIELD_H * 0.85f);
    float y_top = FIELD_H - y_bottom; /* mirror */

    /* P1 drop (bottom court) */
    ps->drop1.active = true;
    ps->drop1.type = t;
    ps->drop1.x = x;
    ps->drop1.y = y_bottom;
    ps->drop1.life = 8.0f;
    ps->drop1.pulse = 0;
    ps->drop1.owner = 1;

    /* P2 drop (top court, mirrored X and Y) */
    ps->drop2.active = true;
    ps->drop2.type = t;
    ps->drop2.x = FIELD_W - x; /* mirror X for fairness */
    ps->drop2.y = y_top;
    ps->drop2.life = 8.0f;
    ps->drop2.pulse = 0;
    ps->drop2.owner = 2;
}

/* ---------- public API ---------- */

void powerup_init(PowerUpState *ps)
{
    for (size_t i = 0; i < sizeof(*ps); i++)
        ((char *)ps)[i] = 0;
    ps->spawn_interval = 12.0f;
    ps->spawn_timer = 8.0f;
    ps->drop1.active = false;
    ps->drop2.active = false;
}

void powerup_update(PowerUpState *ps,
                    GameState *g, float dt,
                    int mode_is_host_or_local)
{
    ps->ev_pickup = false;
    ps->ev_tie = false;

    /* tick effect timers */
    if (ps->fx.grow1 > 0)
        ps->fx.grow1 -= dt;
    if (ps->fx.grow2 > 0)
        ps->fx.grow2 -= dt;
    if (ps->fx.slow > 0)
        ps->fx.slow -= dt;
    if (ps->fx.freeze1 > 0)
        ps->fx.freeze1 -= dt;
    if (ps->fx.freeze2 > 0)
        ps->fx.freeze2 -= dt;

    if (!mode_is_host_or_local)
        return;

    /* freeze paddles */
    if (ps->fx.freeze1 > 0)
    {
        g->p1.vx = g->p1.vy = 0;
    }
    if (ps->fx.freeze2 > 0)
    {
        g->p2.vx = g->p2.vy = 0;
    }

    /* ----- spawn timer ----- */
    bool any_active = ps->drop1.active || ps->drop2.active;
    if (!any_active)
    {
        ps->spawn_timer -= dt;
        if (ps->spawn_timer <= 0)
        {
            ps->spawn_timer = ps->spawn_interval;
            spawn_pair(ps);
        }
        return;
    }

    /* ----- tick lifetime for both ----- */
    if (ps->drop1.active)
    {
        ps->drop1.life -= dt;
        ps->drop1.pulse += dt;
        if (ps->drop1.life <= 0)
            ps->drop1.active = false;
    }
    if (ps->drop2.active)
    {
        ps->drop2.life -= dt;
        ps->drop2.pulse += dt;
        if (ps->drop2.life <= 0)
            ps->drop2.active = false;
    }

    /* ----- pickup detection (both in same frame for tie) ----- */
    float r_pick = 22.0f;

    bool p1_grabbed = false;
    bool p2_grabbed = false;

    if (ps->drop1.active)
    {
        float dx = g->p1.x - ps->drop1.x;
        float dy = g->p1.y - ps->drop1.y;
        float rr = (r_pick + PADDLE_R);
        if (dx * dx + dy * dy < rr * rr)
            p1_grabbed = true;
    }

    if (ps->drop2.active)
    {
        float dx = g->p2.x - ps->drop2.x;
        float dy = g->p2.y - ps->drop2.y;
        float rr = (r_pick + PADDLE_R);
        if (dx * dx + dy * dy < rr * rr)
            p2_grabbed = true;
    }

    /* ----- TIE: both grabbed same frame ----- */
    if (p1_grabbed && p2_grabbed)
    {
        ps->drop1.active = false;
        ps->drop2.active = false;
        ps->ev_tie = true;
        return;
    }

    /* ----- P1 wins ----- */
    if (p1_grabbed)
    {
        apply_powerup(ps, ps->drop1.type, 1);
        ps->drop1.active = false;
        ps->drop2.active = false; /* other vanishes */
        ps->p1_pickups++;
        ps->ev_pickup = true;
        ps->ev_pickup_player = 1;
        ps->ev_pickup_type = ps->drop1.type;
        return;
    }

    /* ----- P2 wins ----- */
    if (p2_grabbed)
    {
        apply_powerup(ps, ps->drop2.type, 2);
        ps->drop1.active = false;
        ps->drop2.active = false;
        ps->p2_pickups++;
        ps->ev_pickup = true;
        ps->ev_pickup_player = 2;
        ps->ev_pickup_type = ps->drop2.type;
        return;
    }
}

static void apply_powerup(PowerUpState *ps, PowerUpType type, int player)
{
    switch (type)
    {
    case PU_GROW:
        if (player == 1)
            ps->fx.grow1 += 8.0f;
        else
            ps->fx.grow2 += 8.0f;
        break;
    case PU_SLOW:
        ps->fx.slow += 5.0f;
        break;
    case PU_FREEZE:
        if (player == 1)
            ps->fx.freeze2 += 2.0f;
        else
            ps->fx.freeze1 += 2.0f;
        break;
    default:
        break;
    }
}

/* ---------- queries ---------- */

float powerup_paddle_radius(const PowerUpState *ps, int player)
{
    if (player == 1 && ps->fx.grow1 > 0)
        return PADDLE_R * 1.5f;
    if (player == 2 && ps->fx.grow2 > 0)
        return PADDLE_R * 1.5f;
    return PADDLE_R;
}

float powerup_puck_speed_scale(const PowerUpState *ps)
{
    return (ps->fx.slow > 0) ? 0.4f : 1.0f;
}

bool powerup_paddle_frozen(const PowerUpState *ps, int player)
{
    if (player == 1 && ps->fx.freeze1 > 0)
        return true;
    if (player == 2 && ps->fx.freeze2 > 0)
        return true;
    return false;
}

/* ---------- drawing ---------- */

static Color color_for(PowerUpType t)
{
    switch (t)
    {
    case PU_GROW:
        return (Color){100, 255, 140, 255};
    case PU_SLOW:
        return (Color){100, 180, 255, 255};
    case PU_FREEZE:
        return (Color){255, 100, 180, 255};
    default:
        return WHITE;
    }
}

static const char *icon_for(PowerUpType t)
{
    switch (t)
    {
    case PU_GROW:
        return "G";
    case PU_SLOW:
        return "S";
    case PU_FREEZE:
        return "F";
    default:
        return "?";
    }
}

static void draw_drop(const PowerUpDrop *d, int offset_y)
{
    if (!d->active)
        return;

    int x = (int)d->x;
    int y = (int)d->y + offset_y;

    float pulse = 0.9f + 0.1f * sinf(d->pulse * 6.0f);
    float r = 22.0f * pulse;
    Color c = color_for(d->type);

    Color glow = c;
    glow.a = 80;
    DrawCircle(x, y, r + 8, glow);
    DrawCircle(x, y, r, c);
    DrawCircleLines(x, y, r, RAYWHITE);

    const char *icon = icon_for(d->type);
    int tw = MeasureText(icon, 26);
    DrawText(icon, x - tw / 2, y - 13, 26, RAYWHITE);

    if (d->life < 3.0f)
    {
        float frac = d->life / 3.0f;
        DrawCircleLines(x, y, r + 4,
                        (Color){255, 255, 255, (unsigned char)(180 * frac)});
    }
}

void powerup_draw(const PowerUpState *ps, int offset_y)
{
    draw_drop(&ps->drop1, offset_y);
    draw_drop(&ps->drop2, offset_y);
}