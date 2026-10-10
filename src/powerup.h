#ifndef POWERUP_H
#define POWERUP_H

#include "game.h"
#include "raylib.h"
#include <stdbool.h>

typedef enum
{
    PU_NONE = 0,
    PU_GROW,
    PU_SLOW,
    PU_FREEZE
} PowerUpType;

typedef struct
{
    bool active;
    PowerUpType type;
    float x, y;
    float life;  /* seconds remaining before despawn */
    float pulse; /* animation phase */
    int owner;   /* 1 or 2 — which player's court it's on */
} PowerUpDrop;

typedef struct
{
    float grow1, grow2;
    float slow;
    float freeze1, freeze2;
} PowerUpEffects;

typedef struct
{
    /* TWO drops now — one per court */
    PowerUpDrop drop1;
    PowerUpDrop drop2;

    PowerUpEffects fx;

    float spawn_timer;
    float spawn_interval;

    int p1_pickups;
    int p2_pickups;

    /* Events */
    bool ev_pickup;
    int ev_pickup_player;
    PowerUpType ev_pickup_type;
    bool ev_tie; /* both grabbed same frame */
} PowerUpState;

/* API */
void powerup_init(PowerUpState *ps);
void powerup_update(PowerUpState *ps,
                    GameState *g, float dt,
                    int mode_is_host_or_local);
void powerup_draw(const PowerUpState *ps, int offset_y);

float powerup_paddle_radius(const PowerUpState *ps, int player);
float powerup_puck_speed_scale(const PowerUpState *ps);
bool powerup_paddle_frozen(const PowerUpState *ps, int player);

#endif