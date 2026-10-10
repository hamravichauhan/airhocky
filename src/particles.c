#include "particles.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"

static Particle s_particles[MAX_PARTICLES];
static Trail s_trail[MAX_TRAIL];
static int s_trail_head = 0;

/* ---------- helpers ---------- */

static float randf(void)
{
    return (float)rand() / (float)RAND_MAX;
}

static float rand_range(float lo, float hi)
{
    return lo + randf() * (hi - lo);
}

static Particle *alloc_particle(void)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        if (!s_particles[i].active)
            return &s_particles[i];
    }
    /* If full, overwrite the oldest (index 0 is fine for our scale) */
    return &s_particles[0];
}

/* ---------- public API ---------- */

void particles_init(void)
{
    memset(s_particles, 0, sizeof(s_particles));
    memset(s_trail, 0, sizeof(s_trail));
    s_trail_head = 0;
}

void particles_clear(void)
{
    particles_init();
}

void particles_update(float dt)
{
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle *p = &s_particles[i];
        if (!p->active)
            continue;

        p->life -= dt;
        if (p->life <= 0)
        {
            p->active = false;
            continue;
        }

        p->pos.x += p->vel.x * dt;
        p->pos.y += p->vel.y * dt;

        /* friction */
        float fr = (p->type == PT_CONFETTI) ? 1.5f : 4.0f;
        p->vel.x -= p->vel.x * fr * dt;
        p->vel.y -= p->vel.y * fr * dt;

        /* confetti has slight gravity */
        if (p->type == PT_CONFETTI)
        {
            p->vel.y += 180.0f * dt;
            p->rotation += p->rot_speed * dt;
        }
    }

    /* trail decay */
    for (int i = 0; i < MAX_TRAIL; i++)
    {
        if (s_trail[i].life > 0)
            s_trail[i].life -= dt * 3.0f;
    }
}

void particles_draw(int offset_y)
{
    /* --- trail (draw first, behind everything) --- */
    for (int i = 0; i < MAX_TRAIL; i++)
    {
        Trail *t = &s_trail[i];
        if (t->life <= 0)
            continue;
        float a = t->life;
        if (a > 1.0f)
            a = 1.0f;
        unsigned char alpha = (unsigned char)(80 * a);
        DrawCircle((int)t->pos.x, (int)t->pos.y + offset_y,
                   (int)(PUCK_R * (0.5f + 0.5f * a)),
                   (Color){255, 220, 100, alpha});
    }

    /* --- particles --- */
    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        Particle *p = &s_particles[i];
        if (!p->active)
            continue;

        float a = p->life / p->max_life;
        if (a > 1.0f)
            a = 1.0f;

        Color c = p->color;
        c.a = (unsigned char)(c.a * a);

        int px = (int)p->pos.x;
        int py = (int)p->pos.y + offset_y;

        if (p->type == PT_CONFETTI)
        {
            DrawRectanglePro(
                (Rectangle){(float)px, (float)py, p->size, p->size},
                (Vector2){p->size / 2, p->size / 2},
                p->rotation * RAD2DEG, c);
        }
        else
        {
            DrawCircle(px, py, p->size, c);
        }
    }
}

/* ---------- emitters ---------- */

void particles_emit_spark(Vector2 pos, Vector2 direction, float intensity)
{
    if (intensity < 0)
        intensity = 0;
    if (intensity > 1)
        intensity = 1;

    int count = 8 + (int)(intensity * 18);

    for (int i = 0; i < count; i++)
    {
        Particle *p = alloc_particle();
        float ang = atan2f(direction.y, direction.x) + rand_range(-1.0f, 1.0f);
        float speed = rand_range(120.0f, 400.0f) * (0.6f + intensity);

        p->pos = pos;
        p->vel.x = cosf(ang) * speed;
        p->vel.y = sinf(ang) * speed;
        p->max_life = rand_range(0.25f, 0.5f);
        p->life = p->max_life;
        p->size = rand_range(1.5f, 3.5f);
        p->color = (Color){
            (unsigned char)rand_range(200, 255),
            (unsigned char)rand_range(180, 230),
            (unsigned char)rand_range(80, 160),
            255};
        p->type = PT_SPARK;
        p->active = true;
    }
}

void particles_emit_dust(Vector2 pos, int count)
{
    for (int i = 0; i < count; i++)
    {
        Particle *p = alloc_particle();
        float ang = rand_range(0, 2 * PI);
        float speed = rand_range(30.0f, 110.0f);
        p->pos = pos;
        p->vel.x = cosf(ang) * speed;
        p->vel.y = sinf(ang) * speed;
        p->max_life = rand_range(0.35f, 0.7f);
        p->life = p->max_life;
        p->size = rand_range(2.0f, 4.0f);
        p->color = (Color){200, 200, 200, 200};
        p->type = PT_DUST;
        p->active = true;
    }
}

void particles_emit_goal(Vector2 pos, Color theme_color)
{
    (void)theme_color;
    Color palette[5] = {
        (Color){255, 100, 100, 255},
        (Color){100, 180, 255, 255},
        (Color){255, 220, 80, 255},
        (Color){120, 255, 140, 255},
        (Color){255, 120, 220, 255}};

    for (int i = 0; i < 90; i++)
    {
        Particle *p = alloc_particle();
        float ang = rand_range(0, 2 * PI);
        float speed = rand_range(80.0f, 480.0f);

        p->pos = pos;
        p->vel.x = cosf(ang) * speed;
        p->vel.y = sinf(ang) * speed - 100.0f; /* upward bias */
        p->max_life = rand_range(0.8f, 1.6f);
        p->life = p->max_life;
        p->size = rand_range(4.0f, 8.0f);
        p->color = palette[rand() % 5];
        p->rotation = rand_range(0, 2 * PI);
        p->rot_speed = rand_range(-6.0f, 6.0f);
        p->type = PT_CONFETTI;
        p->active = true;
    }
}

void particles_emit_ring(Vector2 pos, Color color)
{
    /* Ring = 24 sparks arranged in a circle, moving outward */
    for (int i = 0; i < 24; i++)
    {
        Particle *p = alloc_particle();
        float ang = (2 * PI * i) / 24.0f;
        float speed = 350.0f;

        p->pos = pos;
        p->vel.x = cosf(ang) * speed;
        p->vel.y = sinf(ang) * speed;
        p->max_life = 0.4f;
        p->life = p->max_life;
        p->size = 3.0f;
        p->color = color;
        p->type = PT_SPARK;
        p->active = true;
    }
}

/* ---------- trail ---------- */

void particles_trail_add(Vector2 pos)
{
    s_trail[s_trail_head].pos = pos;
    s_trail[s_trail_head].life = 1.0f;
    s_trail_head = (s_trail_head + 1) % MAX_TRAIL;
}

void particles_trail_clear(void)
{
    for (int i = 0; i < MAX_TRAIL; i++)
        s_trail[i].life = 0;
}