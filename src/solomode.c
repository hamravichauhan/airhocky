#include "solomode.h"
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
        ai->reaction_delay = 0.00f;
        ai->max_speed = 1600.0f;
        ai->jitter = 0.0f;
        ai->prediction_quality = 1.00f;
        ai->attack_aggression = 1.00f;
        ai->strike_threshold = 900.0f;
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
   Prediction
   ============================================================ */
static float predict_puck_at_y(const GameState *g, float target_y,
                               float max_time, float *out_x)
{
    float px = g->puck.x;
    float py = g->puck.y;
    float vx = g->puck.vx;
    float vy = g->puck.vy;

    if (fabsf(vy) < 15.0f)
    {
        *out_x = px;
        return -1.0f;
    }

    float dy = target_y - py;
    if (dy * vy <= 0)
    {
        *out_x = px;
        return -1.0f;
    }

    float t = dy / vy;
    if (t < 0)
        t = -t;
    if (t > max_time)
        t = max_time;

    float left = WALL_THICK + PUCK_R;
    float right = FIELD_W - WALL_THICK - PUCK_R;
    float span = right - left;

    float fx = px + vx * t;

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
    /* reaction delay tick */
    if (ai->reaction_timer > 0)
    {
        ai->reaction_timer -= dt;
        if (ai->reaction_timer > 0)
            return;
    }

    /* perceived puck position (with prediction error) */
    float err_scale = (1.0f - ai->prediction_quality) * 100.0f;
    float perceived_px = g->puck.x + rand_unit() * err_scale * 0.15f;
    float perceived_py = g->puck.y + rand_unit() * err_scale * 0.15f;

    const float DEFENSE_LINE_Y = 90.0f;
    const float MIDLINE = FIELD_H / 2.0f;
    const float HOME_Y = 130.0f;
    const float GOAL_CX = FIELD_W / 2.0f;

    float puck_speed = sqrtf(g->puck.vx * g->puck.vx +
                             g->puck.vy * g->puck.vy);

    bool puck_on_my_half = (perceived_py < MIDLINE);
    bool puck_approaching = (g->puck.vy < -30.0f);
    bool puck_slow = (puck_speed < ai->strike_threshold);

    float desired_x = ai->target_x;
    float desired_y = ai->target_y;
    AIState new_state = ai->state;

    /* ================================================================
       STATE MACHINE
       ================================================================ */

    /* ---- STRIKE ---- */
    bool can_strike = puck_on_my_half && puck_slow &&
                      perceived_py > 60.0f &&
                      perceived_py < MIDLINE - PADDLE_R - 20.0f;

    if (can_strike && ai->attack_aggression > 0.3f)
    {
        new_state = AI_STATE_STRIKE;

        float offset_y = 6.0f;
        float offset_x = 0.0f;

        float target_goal_x = (g->p1.x < GOAL_CX) ? FIELD_W - 120.0f : 120.0f;
        float dx = target_goal_x - perceived_px;
        float dy = FIELD_H - perceived_py;
        float ang = atan2f(dy, dx);
        offset_x = cosf(ang) * (PADDLE_R + PUCK_R + 2.0f);
        offset_y = -sinf(ang) * (PADDLE_R + PUCK_R + 2.0f);

        desired_x = perceived_px - offset_x;
        desired_y = perceived_py - offset_y;

        desired_y = clampf(desired_y,
                           PADDLE_R + WALL_THICK,
                           MIDLINE - PADDLE_R - WALL_THICK);
    }
    /* ---- INTERCEPT ---- */
    else if (puck_approaching || puck_on_my_half)
    {
        new_state = AI_STATE_INTERCEPT;

        float predicted_x = perceived_px;
        float lookahead = 3.0f;
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

        float blend = ai->prediction_quality;
        float intercept_x = predicted_x * blend + perceived_px * (1.0f - blend);

        /* ---- Adaptive bias for HARD ---- */
        if (ai->level == AI_HARD)
        {
            int L = ai->p1_goals_left;
            int R = ai->p1_goals_right;
            int total = L + R + ai->p1_goals_center;
            if (total > 2)
            {
                float bias = 0.0f;
                if (L > R * 2)
                    bias = -40.0f;
                else if (R > L * 2)
                    bias = 40.0f;
                if (ai->p1_hits_counted > 5)
                {
                    bias += (ai->avg_p1_hit_x - FIELD_W / 2.0f) * 0.3f;
                }
                intercept_x += bias;
                if (intercept_x < PADDLE_R + WALL_THICK)
                    intercept_x = PADDLE_R + WALL_THICK;
                if (intercept_x > FIELD_W - PADDLE_R - WALL_THICK)
                    intercept_x = FIELD_W - PADDLE_R - WALL_THICK;
            }
        }

        float defend_y = DEFENSE_LINE_Y;
        if (perceived_py < 200.0f)
        {
            defend_y = perceived_py + PADDLE_R + 4.0f;
        }

        desired_x = intercept_x;
        desired_y = defend_y;
    }
    /* ---- ATTACK ---- */
    else if (ai->attack_aggression > 0.5f && !puck_approaching)
    {
        new_state = AI_STATE_ATTACK;

        desired_x = GOAL_CX + (g->p1.x - GOAL_CX) * 0.55f;
        if (ai->level == AI_HARD)
        {
            desired_y = MIDLINE - PADDLE_R - 40.0f;
        }
        else
        {
            desired_y = HOME_Y + 40.0f * ai->attack_aggression;
        }
    }
    /* ---- RESET ---- */
    else
    {
        new_state = AI_STATE_RESET;
        desired_x = GOAL_CX + (perceived_px - GOAL_CX) * 0.3f;
        desired_y = HOME_Y;
    }

    /* jitter */
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

    /* clamp */
    desired_x = clampf(desired_x, PADDLE_R + WALL_THICK, FIELD_W - PADDLE_R - WALL_THICK);
    desired_y = clampf(desired_y, PADDLE_R + WALL_THICK, MIDLINE - PADDLE_R - WALL_THICK);

    ai->target_x = desired_x;
    ai->target_y = desired_y;
    ai->state = new_state;

    /* reaction timer reset on direction change */
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

void ai_get_target(const AIController *ai, float *tx, float *ty)
{
    *tx = ai->target_x;
    *ty = ai->target_y;
}

void ai_record_player_goal(AIController *ai, float x)
{
    float third = FIELD_W / 3.0f;
    if (x < third)
        ai->p1_goals_left++;
    else if (x > 2 * third)
        ai->p1_goals_right++;
    else
        ai->p1_goals_center++;
}

void ai_record_player_hit(AIController *ai, float x)
{
    ai->p1_hits_counted++;
    float a = ai->avg_p1_hit_x;
    ai->avg_p1_hit_x = a + (x - a) / ai->p1_hits_counted;
}