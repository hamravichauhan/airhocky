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
#define PUCK_FRICTION 0.35f
#define PADDLE_ACCEL 8000.0f
#define PADDLE_MAX_SPEED 1400.0f
#define PADDLE_FRICTION 8.0f
#define PADDLE_MASS 3.0f
#define PUCK_MASS 1.0f
#define COLLISION_RESTITUTION 0.92f
#define MAX_SUBSTEPS 6

/* ----- Game modes ----- */
typedef enum
{
    MODE_LOCAL,
    MODE_SOLO,
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
    Body p1;
    Body p2;
    Body puck;
    float puck_spin;
    int score1;
    int score2;
    GameMode mode;
    bool running;
    float reset_timer;
    float goal_flash;
    int last_scorer;
    float shake;

    /* ----- audio event flags ----- */
    bool ev_hit;
    float ev_hit_intensity;
    bool ev_wall;
    float ev_wall_intensity;
    bool ev_goal;
    int ev_count_tick;
    bool ev_go;

    /* ----- particle event data ----- */
    bool ev_hit_pos_valid;
    float ev_hit_pos_x, ev_hit_pos_y;
    float ev_hit_dir_x, ev_hit_dir_y;

    bool ev_wall_pos_valid;
    float ev_wall_pos_x, ev_wall_pos_y;

    /* ----- power-up modifiers (set by main.c before game_update) ----- */
    float p1_radius_mult; /* 1.0 = normal, 1.5 = grown */
    float p2_radius_mult;
    float puck_speed_mult; /* 1.0 = normal, 0.4 = slowed */
    bool p1_frozen;
    bool p2_frozen;
} GameState;

/* ----- API ----- */
void game_init(GameState *g, GameMode mode);
void game_update(GameState *g, float dt,
                 float p1_tx, float p1_ty,
                 float p2_tx, float p2_ty);
void game_reset_puck(GameState *g, int toward_player);
bool game_check_win(const GameState *g);

#endif