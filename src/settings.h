#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>

typedef enum
{
    SPEED_SLOW = 0,
    SPEED_NORMAL,
    SPEED_FAST,
    SPEED_INSANE
} SpeedLevel;

typedef enum
{
    POWERUP_OFF = 0,
    POWERUP_RARE,
    POWERUP_NORMAL,
    POWERUP_FREQUENT
} PowerUpFreq;

typedef enum
{
    SIZE_SMALL = 0,
    SIZE_NORMAL,
    SIZE_LARGE
} TableSize;

typedef struct
{
    int win_score;          /* 1–21 */
    int rounds;             /* 1–9 */
    float round_time_limit; /* 0 = off, else seconds */

    SpeedLevel puck_speed;
    SpeedLevel paddle_speed;
    PowerUpFreq powerups;
    TableSize table_size;

    int ai_difficulty; /* 0=easy, 1=medium, 2=hard */

    int theme_idx; /* 0..3 */
    int volume;    /* 0–100 */
    bool sound_on;

    /* Last screen state for convenience */
} GameSettings;

void settings_load(GameSettings *s);
void settings_save(const GameSettings *s);

/* Apply settings to runtime globals used by game.c */
void settings_apply(const GameSettings *s);

/* Helpers for display */
const char *settings_speed_name(SpeedLevel s);
const char *settings_powerup_name(PowerUpFreq p);
const char *settings_table_name(TableSize t);

#endif