#include "settings.h"
#include "game.h"
#include "themes.h"
#include <stdio.h>
#include <string.h>

/* ------------ Runtime globals (read by game.c) ------------ */
int RT_WIN_SCORE = 7;
float RT_PUCK_MAX_SPEED = 1400.0f;
float RT_PUCK_FRICTION = 0.35f;
float RT_PADDLE_ACCEL = 8000.0f;
float RT_PADDLE_MAX = 1400.0f;
float RT_FIELD_W_MULT = 1.0f;

/* ------------ Defaults ------------ */

static void set_defaults(GameSettings *s)
{
    s->win_score = 7;
    s->rounds = 1;
    s->round_time_limit = 0.0f;
    s->puck_speed = SPEED_NORMAL;
    s->paddle_speed = SPEED_NORMAL;
    s->powerups = POWERUP_NORMAL;
    s->table_size = SIZE_NORMAL;
    s->ai_difficulty = 1;
    s->theme_idx = 0;
    s->volume = 80;
    s->sound_on = true;
}

/* ------------ Load / Save ------------ */

void settings_load(GameSettings *s)
{
    set_defaults(s);

    FILE *f = fopen("settings.dat", "r");
    if (!f)
        return;

    int rc = fscanf(f,
                    "%d\n%d\n%f\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",
                    &s->win_score,
                    &s->rounds,
                    &s->round_time_limit,
                    (int *)&s->puck_speed,
                    (int *)&s->paddle_speed,
                    (int *)&s->powerups,
                    (int *)&s->table_size,
                    &s->ai_difficulty,
                    &s->theme_idx,
                    &s->volume,
                    (int *)&s->sound_on);
    fclose(f);

    if (rc != 11)
        set_defaults(s);

    if (s->win_score < 1)
        s->win_score = 1;
    if (s->win_score > 21)
        s->win_score = 21;
    if (s->rounds < 1)
        s->rounds = 1;
    if (s->rounds > 9)
        s->rounds = 9;
    if (s->volume < 0)
        s->volume = 0;
    if (s->volume > 100)
        s->volume = 100;
}

void settings_save(const GameSettings *s)
{
    FILE *f = fopen("settings.dat", "w");
    if (!f)
        return;
    fprintf(f,
            "%d\n%d\n%.2f\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",
            s->win_score,
            s->rounds,
            s->round_time_limit,
            (int)s->puck_speed,
            (int)s->paddle_speed,
            (int)s->powerups,
            (int)s->table_size,
            s->ai_difficulty,
            s->theme_idx,
            s->volume,
            (int)s->sound_on);
    fclose(f);
}

/* ------------ Apply to runtime globals ------------ */

void settings_apply(const GameSettings *s)
{
    RT_WIN_SCORE = s->win_score;

    /* Puck speed */
    switch (s->puck_speed)
    {
    case SPEED_SLOW:
        RT_PUCK_MAX_SPEED = 700.0f;
        RT_PUCK_FRICTION = 0.50f;
        break;
    case SPEED_FAST:
        RT_PUCK_MAX_SPEED = 2000.0f;
        RT_PUCK_FRICTION = 0.28f;
        break;
    case SPEED_INSANE:
        RT_PUCK_MAX_SPEED = 2800.0f;
        RT_PUCK_FRICTION = 0.20f;
        break;
    case SPEED_NORMAL:
    default:
        RT_PUCK_MAX_SPEED = 1400.0f;
        RT_PUCK_FRICTION = 0.35f;
        break;
    }

    /* Paddle speed */
    switch (s->paddle_speed)
    {
    case SPEED_SLOW:
        RT_PADDLE_ACCEL = 4000.0f;
        RT_PADDLE_MAX = 800.0f;
        break;
    case SPEED_FAST:
        RT_PADDLE_ACCEL = 12000.0f;
        RT_PADDLE_MAX = 1800.0f;
        break;
    case SPEED_INSANE:
        RT_PADDLE_ACCEL = 16000.0f;
        RT_PADDLE_MAX = 2400.0f;
        break;
    case SPEED_NORMAL:
    default:
        RT_PADDLE_ACCEL = 8000.0f;
        RT_PADDLE_MAX = 1400.0f;
        break;
    }

    /* Theme */
    themes_set_by_index(s->theme_idx);
}

/* ------------ Display helpers ------------ */

const char *settings_speed_name(SpeedLevel s)
{
    switch (s)
    {
    case SPEED_SLOW:
        return "Slow";
    case SPEED_NORMAL:
        return "Normal";
    case SPEED_FAST:
        return "Fast";
    case SPEED_INSANE:
        return "Insane";
    }
    return "?";
}

const char *settings_powerup_name(PowerUpFreq p)
{
    switch (p)
    {
    case POWERUP_OFF:
        return "Off";
    case POWERUP_RARE:
        return "Rare";
    case POWERUP_NORMAL:
        return "Normal";
    case POWERUP_FREQUENT:
        return "Frequent";
    }
    return "?";
}

const char *settings_table_name(TableSize t)
{
    switch (t)
    {
    case SIZE_SMALL:
        return "Small";
    case SIZE_NORMAL:
        return "Normal";
    case SIZE_LARGE:
        return "Large";
    }
    return "?";
}