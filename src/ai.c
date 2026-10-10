#include "ai.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
   Difficulty presets
   ============================================================ */
static void apply_preset(AIController *ai, AIDifficulty level)
{
    ai->level = level;
    switch (level)
    {
    case AI_EASY:
        ai->reaction_delay = 0.30f;
        ai->max_speed = 500.0f;
        ai->jitter = 55.0f;
        ai->prediction_quality = 0.35f;
        ai->attack_aggression = 0.15f;
        ai->strike_threshold = 200.0f;
        break;
    case AI_MEDIUM:
        ai->reaction_delay = 0.14f;
        ai->max_speed = 780.0f;
        ai->jitter = 22.0f;
        ai->prediction_quality = 0.72f;
        ai->attack_aggression = 0.55f;
        ai->strike_threshold = 350.0f;
        break;
    case AI_HARD:
        ai->reaction_delay = 0.00f;     /* instant */
        ai->max_speed = 1600.0f;        /* faster than puck max */
        ai->jitter = 0.0f;              /* perfect aim */
        ai->prediction_quality = 1.00f; /* perfect read */
        ai->attack_aggression = 1.00f;  /* always attacks when possible */
        ai->strike_threshold = 900.0f;  /* attacks even moderately fast pucks */
        break;
    }
}

void ai_init(AIController *ai, AIDifficulty level)
{
    memset(ai, 0, sizeof(*ai));
    apply_preset(ai, level);
    ai->target_x = FIELD_W / 2.0f;
    ai->target_y = 130.0f;
    ai->state = AI_STATE_RESET;
}

const char *ai_state_name(AIState s)
{
    switch (s)
    {
    case AI_STATE_DEFEND:
        return "DEFEND";
    case AI_STATE_INTERCEPT:
        return "INTERCEPT";
    case AI_STATE_ATTACK:
        return "ATTACK";
    case AI_STATE_STRIKE:
        return "STRIKE";
    case AI_STATE_RESET:
        return "RESET";
    }
    return "?";
}

/* ============================================================
   Prediction — trace puck forward with wall bounces
   Returns y position of puck when it reaches target_y, and writes
   the x at that moment into *out_x.
   Handles multiple wall bounces.
   ============================================================ */
static float predict_puck_at_y(const GameState *g, float target_y,
                               float max_time, float *out_x)
{
    float px = g->puck.x;
    float py = g->puck.y;
    float vx = g->puck.vx;
    float vy = g->puck.vy;

    /* If puck is not moving toward target, return current x */
    if (fabsf(vy) < 15.0f)
    {
        *out_x = px;
        return -1.0f;
    }

    float dy = target_y - py;
    if (dy * vy <= 0)
    {
        /* target is behind puck's motion */
        *out_x = px;
        return -1.0f;
    }

    float t = dy / vy;
    if (t < 0)
        t = -t;
    if (t > max_time)
        t = max_time;

    /* Extrapolate x with bounces */
    float left = WALL_THICK + PUCK_R;
    float right = FIELD_W - WALL_THICK - PUCK_R;
    float span = right - left;

    float fx = px + vx * t;

    /* Fold into [left, right] using triangle wave */
    float rel = fx - left;
    float period = 2.0f * span;
    rel = fmodf(rel, period);
    if (rel < 0)
        rel += period;
    if (rel > span)
        rel = period - rel;
    fx = left + rel;

    *out_x = fx;
    return t;
}

/* ============================================================
   Helpers
   ============================================================ */

static float rand_unit(void)
{
    return ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
}

static float clampf(float v, float lo, float hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

/* ============================================================
   Main AI decision
   ============================================================ */

void ai_update(AIController *ai, const GameState *g, float dt)
{
    /* -------- reaction delay tick -------- */
    if (ai->reaction_timer > 0)
    {
        ai->reaction_timer -= dt;
        if (ai->reaction_timer > 0)
            return; /* keep old target */
    }

    /* -------- perceived puck position (with prediction error) -------- */
    /* Higher prediction_quality → more accurate reading */
    float err_scale = (1.0f - ai->prediction_quality) * 100.0f;
    float perceived_px = g->puck.x + rand_unit() * err_scale * 0.15f;
    float perceived_py = g->puck.y + rand_unit() * err_scale * 0.15f;

    /* -------- constants -------- */
    const float DEFENSE_LINE_Y = 90.0f; /* just below top wall */
    const float MIDLINE = FIELD_H / 2.0f;
    const float HOME_Y = 130.0f;
    const float GOAL_CX = FIELD_W / 2.0f;

    float puck_speed = sqrtf(g->puck.vx * g->puck.vx +
                             g->puck.vy * g->puck.vy);

    bool puck_on_my_half = (perceived_py < MIDLINE);
    bool puck_approaching = (g->puck.vy < -30.0f); /* heading toward AI goal */
    bool puck_slow = (puck_speed < ai->strike_threshold);

    float desired_x = ai->target_x;
    float desired_y = ai->target_y;
    AIState new_state = ai->state;

    /* ================================================================
       STATE MACHINE
       ================================================================ */

    /* ---- STRIKE: puck is on my half, slow, and I can reach it ---- */
    bool can_strike = puck_on_my_half && puck_slow &&
                      perceived_py > 60.0f &&
                      perceived_py < MIDLINE - PADDLE_R - 20.0f;

    if (can_strike && ai->attack_aggression > 0.3f)
    {
        /* Aim paddle so it hits puck toward player's goal */
        /* We want the puck to travel down toward player's goal area.
           Paddle should approach from the "top" of the puck (higher y),
           so the puck gets pushed downward. */

        new_state = AI_STATE_STRIKE;

        /* Target: puck position + slight offset to control direction */
        float offset_y = 6.0f; /* slightly above puck (in screen terms) */
        float offset_x = 0.0f;

        /* Aim toward opposite corner of player's goal for a nicer shot */
        float target_goal_x = (g->p1.x < GOAL_CX) ? FIELD_W - 120.0f : 120.0f;
        float dx = target_goal_x - perceived_px;
        float dy = FIELD_H - perceived_py;
        float ang = atan2f(dy, dx);
        /* Push paddle to a spot that sends puck along (ang) */
        offset_x = cosf(ang) * (PADDLE_R + PUCK_R + 2.0f);
        offset_y = -sinf(ang) * (PADDLE_R + PUCK_R + 2.0f);

        desired_x = perceived_px - offset_x;
        desired_y = perceived_py - offset_y;

        /* Clamp Y to AI's half */
        desired_y = clampf(desired_y,
                           PADDLE_R + WALL_THICK,
                           MIDLINE - PADDLE_R - WALL_THICK);
    }
    /* ---- INTERCEPT: puck heading toward AI, predict landing ---- */
    else if (puck_approaching || puck_on_my_half)
    {
        new_state = AI_STATE_INTERCEPT;

        /* Predict where puck crosses defense line */
        float predicted_x = perceived_px;
        float lookahead = 3.0f; /* seconds — enough for multiple bounces */

        /* Adjust lookahead based on difficulty (Hard thinks further ahead) */
        if (ai->level == AI_EASY)
            lookahead = 1.5f;
        if (ai->level == AI_MEDIUM)
            lookahead = 2.2f;
        if (ai->level == AI_HARD)
            lookahead = 4.0f;

        float t_arrive = 0;
        float hit_x = predict_puck_at_y(g, DEFENSE_LINE_Y, lookahead, &predicted_x);
        (void)t_arrive;
        (void)hit_x;

        /* Blend prediction with raw position based on quality */
        float blend = ai->prediction_quality;
        float intercept_x = predicted_x * blend + perceived_px * (1.0f - blend);

        /* Defend near the goal mouth — bias toward goal center a bit */
        float defend_y = DEFENSE_LINE_Y;

        /* If puck is close, move toward it for a proper hit */
        if (perceived_py < 200.0f)
        {
            defend_y = perceived_py + PADDLE_R + 4.0f;
        }

        desired_x = intercept_x;
        desired_y = defend_y;
    }
    /* ---- ATTACK: puck is far on player's half, pre-position ---- */
    else if (ai->attack_aggression > 0.5f && !puck_approaching)
    {
        new_state = AI_STATE_ATTACK;

        /* Sit near midfield, mirror player's x movement.
   Hard AI presses forward — sits higher up, close to midfield. */
        desired_x = GOAL_CX + (g->p1.x - GOAL_CX) * 0.55f;
        if (ai->level == AI_HARD)
        {
            desired_y = MIDLINE - PADDLE_R - 40.0f; /* right at midfield */
        }
        else
        {
            desired_y = HOME_Y + 40.0f * ai->attack_aggression;
        }
    }
    /* ---- RESET: default ---- */
    else
    {
        new_state = AI_STATE_RESET;
        desired_x = GOAL_CX + (perceived_px - GOAL_CX) * 0.3f;
        desired_y = HOME_Y;
    }

    /* ================================================================
       Apply jitter (difficulty personality)
       ================================================================ */
    if (ai->jitter > 0.5f)
    {
        ai->jitter_x = rand_unit() * ai->jitter;
        ai->jitter_y = rand_unit() * ai->jitter * 0.6f;
    }
    else
    {
        ai->jitter_x = 0;
        ai->jitter_y = 0;
    }

    desired_x += ai->jitter_x;
    desired_y += ai->jitter_y;

    /* ================================================================
       Clamp to AI's half + boundaries
       ================================================================ */
    desired_x = clampf(desired_x, PADDLE_R + WALL_THICK, FIELD_W - PADDLE_R - WALL_THICK);
    desired_y = clampf(desired_y, PADDLE_R + WALL_THICK, MIDLINE - PADDLE_R - WALL_THICK);

    ai->target_x = desired_x;
    ai->target_y = desired_y;
    ai->state = new_state;

    /* ================================================================
        Reaction timer — reset when puck direction flips
        ================================================================ */
    float vy_now = g->puck.vy;
    if ((ai->last_puck_y < 0 && vy_now > 0) ||
        (ai->last_puck_y > 0 && vy_now < 0))
    {
        ai->reaction_timer = ai->reaction_delay;
    }
    ai->last_puck_y = vy_now;
    ai->last_puck_x = g->puck.x;

    (void)puck_slow;
}

/* ============================================================
   Public getter
   ============================================================ */
void ai_get_target(const AIController *ai, float *tx, float *ty)
{
    *tx = ai->target_x;
    *ty = ai->target_y;
}