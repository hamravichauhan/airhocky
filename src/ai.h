#ifndef AI_H
#define AI_H

#include "game.h"

typedef enum
{
    AI_EASY,
    AI_MEDIUM,
    AI_HARD
} AIDifficulty;

typedef enum
{
    AI_STATE_DEFEND,
    AI_STATE_INTERCEPT,
    AI_STATE_ATTACK,
    AI_STATE_STRIKE,
    AI_STATE_RESET
} AIState;

typedef struct
{
    AIDifficulty level;

    /* Difficulty knobs */
    float reaction_delay;     /* seconds */
    float max_speed;          /* units/sec */
    float jitter;             /* px */
    float prediction_quality; /* 0..1 — how accurate the prediction is */
    float attack_aggression;  /* 0..1 — how willing to push forward */
    float strike_threshold;   /* puck speed below which AI will attack */

    /* Runtime */
    float reaction_timer;
    float target_x, target_y;
    float jitter_x, jitter_y;

    /* Behavior */
    AIState state;
    float state_timer;
    float last_puck_x, last_puck_y;

    /* Strike windup */
    float strike_x, strike_y;
    bool striking;
} AIController;

void ai_init(AIController *ai, AIDifficulty level);
void ai_update(AIController *ai, const GameState *g, float dt);
void ai_get_target(const AIController *ai, float *tx, float *ty);
const char *ai_state_name(AIState s);

#endif