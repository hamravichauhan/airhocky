#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

/* ----- Field dimensions ----- */
#define FIELD_W 600
#define FIELD_H 800
#define WALL_THICK 10
#define PADDLE_R 28
#define PUCK_R 14
#define GOAL_WIDTH 180
#define WIN_SCORE 7

/* ----- Physics tuning ----- */
#define PUCK_MAX_SPEED 1400.0f
#define PUCK_MIN_SPEED 10.0f
#define PUCK_FRICTION 0.35f /* per second */
#define PADDLE_ACCEL 4500.0f
#define PADDLE_MAX_SPEED 900.0f
#define PADDLE_FRICTION 8.0f
#define PADDLE_MASS 3.0f
#define PUCK_MASS 1.0f
#define COLLISION_RESTITUTION 0.92f
#define MAX_SUBSTEPS 6

/* ----- Game modes ----- */
typedef enum
{
    MODE_LOCAL,
    MODE_HOST,
    MODE_CLIENT
} GameMode;

/* ----- Objects ----- */
typedef struct
{
    float x, y;
    float vx, vy;
} Body;

typedef struct
{
    Body p1; /* bottom paddle (host) */
    Body p2; /* top paddle    (client) */
    Body puck;
    float puck_spin; /* angular velocity (rad/s) — cosmetic + slight curve */
    int score1;
    int score2;
    GameMode mode;
    bool running;
    float reset_timer; /* countdown before puck relaunches */
    float goal_flash;  /* visual flash duration after a goal */
    int last_scorer;   /* 1 or 2 — who scored last (for effects) */
    float shake;       /* screen shake intensity */
} GameState;

/* ----- API ----- */
void game_init(GameState *g, GameMode mode);
void game_update(GameState *g, float dt,
                 float p1_tx, float p1_ty,
                 float p2_tx, float p2_ty);
void game_reset_puck(GameState *g, int toward_player);
bool game_check_win(const GameState *g);

#endif