#include "settings_ui.h"
#include "raylib.h"
#include "features.h"
#include <stdio.h>
#include <math.h>
#include "game.h"
#include "ui.h"

#define SETTINGS_ROWS 10

static int s_sel = 0;

static const char *row_labels[SETTINGS_ROWS] = {
    "Win Score",
    "Rounds per Match",
    "Round Time Limit",
    "Puck Speed",
    "Paddle Speed",
    "Power-Ups",
    "Table Size",
    "AI Difficulty",
    "Theme",
    "Sound Volume"};

static void clamp_sel(void)
{
    if (s_sel < 0)
        s_sel = 0;
    if (s_sel >= SETTINGS_ROWS)
        s_sel = SETTINGS_ROWS - 1;
}

static void adjust(GameSettings *s, int dir)
{
    switch (s_sel)
    {
    case 0:
        s->win_score += dir;
        if (s->win_score < 1)
            s->win_score = 1;
        if (s->win_score > 21)
            s->win_score = 21;
        break;
    case 1:
        s->rounds += dir;
        if (s->rounds < 1)
            s->rounds = 1;
        if (s->rounds > 9)
            s->rounds = 9;
        break;
    case 2:
    {
        float opts[5] = {0.0f, 30.0f, 60.0f, 120.0f, 300.0f};
        int idx = 0;
        for (int i = 0; i < 5; i++)
            if (opts[i] == s->round_time_limit)
                idx = i;
        idx += dir;
        if (idx < 0)
            idx = 0;
        if (idx > 4)
            idx = 4;
        s->round_time_limit = opts[idx];
        break;
    }
    case 3:
        s->puck_speed = (SpeedLevel)((int)s->puck_speed + dir);
        if ((int)s->puck_speed < 0)
            s->puck_speed = 0;
        if ((int)s->puck_speed > 3)
            s->puck_speed = 3;
        break;
    case 4:
        s->paddle_speed = (SpeedLevel)((int)s->paddle_speed + dir);
        if ((int)s->paddle_speed < 0)
            s->paddle_speed = 0;
        if ((int)s->paddle_speed > 3)
            s->paddle_speed = 3;
        break;
    case 5:
        s->powerups = (PowerUpFreq)((int)s->powerups + dir);
        if ((int)s->powerups < 0)
            s->powerups = 0;
        if ((int)s->powerups > 3)
            s->powerups = 3;
        break;
    case 6:
        s->table_size = (TableSize)((int)s->table_size + dir);
        if ((int)s->table_size < 0)
            s->table_size = 0;
        if ((int)s->table_size > 2)
            s->table_size = 2;
        break;
    case 7:
        s->ai_difficulty += dir;
        if (s->ai_difficulty < 0)
            s->ai_difficulty = 0;
        if (s->ai_difficulty > 2)
            s->ai_difficulty = 2;
        break;
    case 8:
        s->theme_idx += dir;
        if (s->theme_idx < 0)
            s->theme_idx = 0;
        if (s->theme_idx > 3)
            s->theme_idx = 3;
        break;
    case 9:
        s->volume += dir * 5;
        if (s->volume < 0)
            s->volume = 0;
        if (s->volume > 100)
            s->volume = 100;
        break;
    }
}

bool settings_ui_update(GameSettings *s)
{
    if (IsKeyPressed(KEY_DOWN))
    {
        s_sel++;
        clamp_sel();
    }
    if (IsKeyPressed(KEY_UP))
    {
        s_sel--;
        clamp_sel();
    }
    if (IsKeyPressed(KEY_LEFT))
        adjust(s, -1);
    if (IsKeyPressed(KEY_RIGHT))
        adjust(s, +1);

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER))
    {
        settings_save(s);
        settings_apply(s);
        return true;
    }
    return false;
}

static const char *fmt_value(const GameSettings *s, int row, char *buf, int n)
{
    switch (row)
    {
    case 0:
        snprintf(buf, n, "%d", s->win_score);
        break;
    case 1:
        snprintf(buf, n, "%d", s->rounds);
        break;
    case 2:
        if (s->round_time_limit <= 0.0f)
            snprintf(buf, n, "Off");
        else
            snprintf(buf, n, "%d sec", (int)s->round_time_limit);
        break;
    case 3:
        snprintf(buf, n, "%s", settings_speed_name(s->puck_speed));
        break;
    case 4:
        snprintf(buf, n, "%s", settings_speed_name(s->paddle_speed));
        break;
    case 5:
        snprintf(buf, n, "%s", settings_powerup_name(s->powerups));
        break;
    case 6:
        snprintf(buf, n, "%s", settings_table_name(s->table_size));
        break;
    case 7:
    {
        const char *names[3] = {"Easy", "Medium", "Hard"};
        int idx = s->ai_difficulty;
        if (idx < 0)
            idx = 0;
        if (idx > 2)
            idx = 2;
        snprintf(buf, n, "%s", names[idx]);
        break;
    }
    case 8:
    {
        const char *names[4] = {"Neon", "Retro", "Dark", "Fire"};
        int idx = s->theme_idx;
        if (idx < 0)
            idx = 0;
        if (idx > 3)
            idx = 3;
        snprintf(buf, n, "%s", names[idx]);
        break;
    }
    case 9:
        snprintf(buf, n, "%d%%", s->volume);
        break;
    default:
        buf[0] = 0;
        break;
    }
    return buf;
}

void settings_ui_draw(const GameSettings *s)
{
    ui_background();

    ui_title("SETTINGS", 0.3f);
    ui_subtitle("Arrows to change values, ENTER/ESC to save + back");

    Rectangle panel = {FIELD_W / 2 - 250, 170, 500, 10 * 40 + 40};
    ui_panel(panel);

    int y = 195;
    for (int i = 0; i < SETTINGS_ROWS; i++)
    {
        Color c = (i == s_sel) ? UI_ACCENT_HOT : UI_TEXT;
        if (i == s_sel)
        {
            float ax = 30 + 5 * sinf(GetTime() * 4.0f);
            DrawTriangle(
                (Vector2){ax, y + 8},
                (Vector2){ax, y + 24},
                (Vector2){ax + 12, y + 16},
                UI_ACCENT_HOT);
        }
        DrawText(row_labels[i], 60, y, 20, c);

        char buf[32];
        fmt_value(s, i, buf, sizeof buf);
        int bw = MeasureText(buf, 20);
        Color vc = (i == s_sel) ? UI_ACCENT_HOT : UI_TEXT;
        DrawText(buf, FIELD_W - 60 - bw, y, 20, vc);

        if (i == s_sel)
        {
            DrawText("<", FIELD_W - 45, y, 20, UI_ACCENT_HOT);
        }

        y += 40;
    }

    ui_hint("ENTER/ESC to save + back");
}