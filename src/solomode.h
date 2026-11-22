#ifndef SOLOMODE_H
#define SOLOMODE_H

#include "game.h"

typedef enum
{
    SOLO_EASY,
    SOLO_MEDIUM,
    SOLO_HARD
} SoloDifficulty;

typedef enum
{
    SOLO_STATE_DEFEND,
    SOLO_STATE_INTERCEPT,
    SOLO_STATE_ATTACK,
    SOLO_STATE_STRIKE,
    SOLO_STATE_RESET
} SoloState;

typedef struct
{
    SoloDifficulty level;

    /* Difficulty knobs */
    float reaction_delay;     /* seconds */
    float max_speed;          /* units/sec */
    float jitter;             /* px */
    float prediction_quality; /* 0..1 — how accurate the prediction is */
    float attack_aggression;  /* 0..1 — how willing to push forward */
    float strike_threshold;   /* puck speed below which solo opponent will attack */

    /* Runtime */
    float reaction_timer;
    float target_x, target_y;
    float jitter_x, jitter_y;

    /* Behavior */
    SoloState state;
    float state_timer;
    float last_puck_x, last_puck_y;

    /* Strike windup */
    float strike_x, strike_y;
    bool striking;

    /* Adaptive learning — where the player scores */
    int p1_goals_left;   /* scored on opponent's left side */
    int p1_goals_right;  /* scored on opponent's right side */
    int p1_goals_center; /* center */
    int p1_shots_total;

    /* Where the player tends to hit from */
    float avg_p1_hit_x;
    int p1_hits_counted;

} SoloController;

void solo_init(SoloController *solo, SoloDifficulty level);
void solo_update(SoloController *solo, const GameState *g, float dt);
void solo_get_target(const SoloController *solo, float *tx, float *ty);
const char *solo_state_name(SoloState s);
void solo_record_player_goal(SoloController *solo, float x);
void solo_record_player_hit(SoloController *solo, float x);

/* Backward compatibility aliases */
typedef SoloDifficulty AIDifficulty;
#define AI_EASY SOLO_EASY
#define AI_MEDIUM SOLO_MEDIUM
#define AI_HARD SOLO_HARD

typedef SoloState AIState;
#define AI_STATE_DEFEND SOLO_STATE_DEFEND
#define AI_STATE_INTERCEPT SOLO_STATE_INTERCEPT
#define AI_STATE_ATTACK SOLO_STATE_ATTACK
#define AI_STATE_STRIKE SOLO_STATE_STRIKE
#define AI_STATE_RESET SOLO_STATE_RESET

typedef SoloController AIController;
#define ai_init solo_init
#define ai_update solo_update
#define ai_get_target solo_get_target
#define ai_state_name solo_state_name
#define ai_record_player_goal solo_record_player_goal
#define ai_record_player_hit solo_record_player_hit

#endif