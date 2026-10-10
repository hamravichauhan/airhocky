#ifndef PARTICLES_H
#define PARTICLES_H

#include "raylib.h"
#include <stdbool.h>

#define MAX_PARTICLES 512
#define MAX_TRAIL 32

typedef enum
{
    PT_SPARK,
    PT_DUST,
    PT_CONFETTI
} ParticleType;

typedef struct
{
    Vector2 pos;
    Vector2 vel;
    float life;
    float max_life;
    float size;
    Color color;
    float rotation;
    float rot_speed;
    ParticleType type;
    bool active;
} Particle;

typedef struct
{
    Vector2 pos;
    float life;
} Trail;

/* --- API --- */
void particles_init(void);
void particles_update(float dt);
void particles_draw(int offset_y);

void particles_emit_spark(Vector2 pos, Vector2 direction, float intensity);
void particles_emit_dust(Vector2 pos, int count);
void particles_emit_goal(Vector2 pos, Color theme_color);
void particles_emit_ring(Vector2 pos, Color color);

void particles_trail_add(Vector2 pos);
void particles_trail_clear(void);

void particles_clear(void);

#endif