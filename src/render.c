#include "render.h"
#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "features.h"
#include "stats.h"

void render_init(void) {}

/* ------- helpers ------- */

static Color lerp_color(Color a, Color b, float t)
{
    if (t < 0)
        t = 0;
    if (t > 1)
        t = 1;
    return (Color){
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        (unsigned char)(a.a + (b.a - a.a) * t),
    };
}

/* ------- main field ------- */

void render_field(const GameState *g)
{
    int top = 100;

    int shx = 0, shy = 0;
    if (g->shake > 0.1f)
    {
        shx = (rand() % 2 ? 1 : -1) * (int)g->shake;
        shy = (rand() % 2 ? 1 : -1) * (int)g->shake;
    }

    DrawRectangleGradientV(0, top, FIELD_W, FIELD_H,
                           (Color){12, 70, 100, 255},
                           (Color){6, 40, 60, 255});

    DrawRectangleLines(0, top, FIELD_W, FIELD_H, (Color){120, 180, 220, 100});

    DrawLine(0, top + FIELD_H / 2, FIELD_W, top + FIELD_H / 2,
             (Color){200, 220, 240, 150});
    DrawCircleLines(FIELD_W / 2, top + FIELD_H / 2, 70,
                    (Color){200, 220, 240, 130});
    DrawCircleLines(FIELD_W / 2, top + FIELD_H / 2, 40,
                    (Color){200, 220, 240, 90});
    DrawCircle(FIELD_W / 2, top + FIELD_H / 2, 5,
               (Color){200, 220, 240, 200});

    int gx = (FIELD_W - GOAL_WIDTH) / 2;

    Color topGoal = RED;
    if (g->last_scorer == 1 && g->goal_flash > 0)
        topGoal = lerp_color(RED, WHITE, g->goal_flash * 2.0f);
    DrawRectangle(gx, top - 6, GOAL_WIDTH, 6, topGoal);
    DrawRectangle(gx, top, GOAL_WIDTH, WALL_THICK, topGoal);

    Color botGoal = BLUE;
    if (g->last_scorer == 2 && g->goal_flash > 0)
        botGoal = lerp_color(BLUE, WHITE, g->goal_flash * 2.0f);
    DrawRectangle(gx, top + FIELD_H - WALL_THICK, GOAL_WIDTH, WALL_THICK, botGoal);
    DrawRectangle(gx, top + FIELD_H, GOAL_WIDTH, 6, botGoal);

    if (g->goal_flash > 0)
    {
        Color wash = (g->last_scorer == 1)
                         ? (Color){255, 60, 60, (unsigned char)(g->goal_flash * 60)}
                         : (Color){60, 120, 255, (unsigned char)(g->goal_flash * 60)};
        DrawRectangle(0, top, FIELD_W, FIELD_H, wash);
    }

    float r1 = PADDLE_R * g->p1_radius_mult;
    float r2 = PADDLE_R * g->p2_radius_mult;

    Color p1_color = g->p1_frozen ? GRAY : BLUE;
    Color p2_color = g->p2_frozen ? GRAY : RED;

    DrawCircle((int)g->p1.x + shx, (int)(g->p1.y + top) + shy,
               r1 + 4, (Color){40, 90, 200, 90});
    DrawCircle((int)g->p1.x + shx, (int)(g->p1.y + top) + shy, r1, p1_color);
    DrawCircleLines((int)g->p1.x + shx, (int)(g->p1.y + top) + shy, r1, RAYWHITE);
    DrawCircle((int)g->p1.x + shx, (int)(g->p1.y + top) + shy, 5, RAYWHITE);

    DrawCircle((int)g->p2.x + shx, (int)(g->p2.y + top) + shy,
               r2 + 4, (Color){200, 50, 50, 90});
    DrawCircle((int)g->p2.x + shx, (int)(g->p2.y + top) + shy, r2, p2_color);
    DrawCircleLines((int)g->p2.x + shx, (int)(g->p2.y + top) + shy, r2, RAYWHITE);
    DrawCircle((int)g->p2.x + shx, (int)(g->p2.y + top) + shy, 5, RAYWHITE);

    DrawCircle((int)g->puck.x + shx, (int)(g->puck.y + top) + shy,
               PUCK_R + 6, (Color){255, 220, 100, 60});
    DrawCircle((int)g->puck.x + shx, (int)(g->puck.y + top) + shy,
               PUCK_R, YELLOW);
    DrawCircleLines((int)g->puck.x + shx, (int)(g->puck.y + top) + shy,
                    PUCK_R, (Color){120, 90, 0, 255});
    DrawCircle((int)g->puck.x + shx, (int)(g->puck.y + top) + shy,
               4, (Color){255, 240, 180, 255});

    DrawRectangle(0, 0, FIELD_W, 100, (Color){15, 20, 30, 255});
    DrawLine(0, 100, FIELD_W, 100, (Color){80, 120, 160, 200});

    char buf[16];
    snprintf(buf, sizeof buf, "%d", g->score1);
    int s1w = MeasureText(buf, 64);
    DrawText(buf, FIELD_W / 4 - s1w / 2, 12, 64, BLUE);

    snprintf(buf, sizeof buf, "%d", g->score2);
    int s2w = MeasureText(buf, 64);
    DrawText(buf, 3 * FIELD_W / 4 - s2w / 2, 12, 64, RED);

    DrawText("P1", FIELD_W / 4 - 14, 78, 16, (Color){140, 170, 210, 255});
    DrawText("P2", 3 * FIELD_W / 4 - 14, 78, 16, (Color){210, 140, 140, 255});

    DrawText(TextFormat("FIRST TO %d", WIN_SCORE),
             FIELD_W / 2 - MeasureText(TextFormat("FIRST TO %d", WIN_SCORE), 16) / 2,
             42, 16, (Color){160, 180, 200, 200});

    if (g->reset_timer > 0)
    {
        int secs = (int)ceilf(g->reset_timer);
        const char *txt;
        Color c = RAYWHITE;
        if (secs >= 3)
            txt = "3";
        else if (secs == 2)
            txt = "2";
        else if (secs == 1)
            txt = "1";
        else
        {
            txt = "GO!";
            c = GREEN;
        }

        int tw = MeasureText(txt, 120);
        float pulse = 0.6f + 0.4f * sinf(GetTime() * 8.0f);
        c.a = (unsigned char)(200 * pulse);

        DrawText(txt, FIELD_W / 2 - tw / 2, top + FIELD_H / 2 - 60, 120, c);
    }
}

/* ------- menu ------- */

void render_menu(int selected)
{
    DrawRectangleGradientV(0, 0, FIELD_W, FIELD_H + 120,
                           (Color){10, 25, 45, 255},
                           (Color){3, 8, 18, 255});

    const char *title = "AIR HOCKEY";
    int tw = MeasureText(title, 56);
    DrawText(title, FIELD_W / 2 - tw / 2, 60, 56, RAYWHITE);
    DrawText("C / SEA Project", FIELD_W / 2 - 70, 118, 16, GRAY);

    const char *items[5] = {
        "Solo vs AI",
        "Local 2-Player",
        "Host LAN Game",
        "Join LAN Game",
        "Quit"};
    for (int i = 0; i < 5; i++)
    {
        Color c = (i == selected) ? YELLOW : LIGHTGRAY;
        int iw = MeasureText(items[i], 30);
        int x = FIELD_W / 2 - iw / 2;
        int y = 210 + i * 55;
        DrawText(items[i], x, y, 30, c);
        if (i == selected)
        {
            DrawText(">", x - 30, y, 30, YELLOW);
            DrawRectangle(x - 34, y + 34, iw + 60, 2, YELLOW);
        }
    }
    DrawText("W/S/Arrows to navigate, ENTER to select",
             FIELD_W / 2 - 190, FIELD_H + 40, 18, GRAY);
}

/* ------- gameover ------- */

void render_gameover(const GameState *g, int winner)
{
    DrawRectangleGradientV(0, 0, FIELD_W, FIELD_H + 120,
                           (Color){10, 25, 45, 255},
                           (Color){3, 8, 18, 255});

    const char *title = (winner == 1) ? "PLAYER 1 WINS!" : "PLAYER 2 WINS!";
    Color c = (winner == 1) ? BLUE : RED;
    int tw = MeasureText(title, 44);
    DrawText(title, FIELD_W / 2 - tw / 2, 200, 44, c);

    char buf[64];
    snprintf(buf, sizeof buf, "%d  -  %d", g->score1, g->score2);
    int sw = MeasureText(buf, 60);
    DrawText(buf, FIELD_W / 2 - sw / 2, 300, 60, RAYWHITE);

    DrawText("Press ENTER for menu",
             FIELD_W / 2 - MeasureText("Press ENTER for menu", 22) / 2,
             460, 22, LIGHTGRAY);
}

void render_waiting_screen(const char *msg)
{
    DrawRectangleGradientV(0, 0, FIELD_W, FIELD_H + 120,
                           (Color){10, 25, 45, 255},
                           (Color){3, 8, 18, 255});
    DrawText(msg, 40, 350, 24, RAYWHITE);

    float t = GetTime();
    int r = 6 + (int)(4 * (0.5f + 0.5f * sinf(t * 4)));
    DrawCircle(30, 358, r, GREEN);
}

void render_solo_difficulty(int hover)
{
    (void)hover;
    DrawRectangleGradientV(0, 0, FIELD_W, FIELD_H + 120,
                           (Color){10, 25, 45, 255},
                           (Color){3, 8, 18, 255});

    const char *title = "SELECT DIFFICULTY";
    int tw = MeasureText(title, 40);
    DrawText(title, FIELD_W / 2 - tw / 2, 120, 40, RAYWHITE);

    const char *items[3] = {
        "1  -  Easy",
        "2  -  Medium",
        "3  -  Hard"};
    const char *descs[3] = {
        "Slow, forgiving, no prediction",
        "Balanced, short lookahead",
        "Fast, predictive, aggressive"};
    Color cols[3] = {GREEN, YELLOW, RED};

    for (int i = 0; i < 3; i++)
    {
        int iw = MeasureText(items[i], 36);
        int x = FIELD_W / 2 - iw / 2;
        int y = 250 + i * 90;
        DrawText(items[i], x, y, 36, cols[i]);

        int dw = MeasureText(descs[i], 18);
        DrawText(descs[i], FIELD_W / 2 - dw / 2, y + 44, 18, GRAY);
    }

    DrawText("1/2/3 to choose   ESC = back",
             FIELD_W / 2 - 130, FIELD_H + 40, 20, LIGHTGRAY);
}

/* ------- combo overlay ------- */

void render_combo_overlay(const MatchStats *s)
{
#if FEATURE_COMBO_COUNTER
    if (s->current_rally < 2 && s->hype_timer <= 0)
        return;

    int top = 100;

    if (s->current_rally >= 2)
    {
        char buf[32];
        snprintf(buf, sizeof buf, "RALLY %d", s->current_rally);

        int fs = 22;
        int tw = MeasureText(buf, fs);

        float intensity = s->current_rally / 30.0f;
        if (intensity > 1.0f)
            intensity = 1.0f;
        unsigned char alpha = (unsigned char)(120 + 100 * intensity);
        Color c = (Color){255, 220, 100, alpha};

        DrawText(buf, FIELD_W / 2 - tw / 2, top + 10, fs, c);
    }

    const char *hype = stats_hype_text(s);
    if (hype)
    {
        float pulse = 0.7f + 0.3f * sinf(s->hype_pulse * 15.0f);
        float fade = s->hype_timer / 1.8f;
        if (fade > 1.0f)
            fade = 1.0f;

        unsigned char a = (unsigned char)(255 * fade * pulse);

        Color c;
        switch (s->hype)
        {
        case HYPE_NICE:
            c = (Color){100, 255, 140, a};
            break;
        case HYPE_GREAT:
            c = (Color){100, 200, 255, a};
            break;
        case HYPE_INSANE:
            c = (Color){255, 180, 80, a};
            break;
        case HYPE_LEGENDARY:
            c = (Color){255, 80, 180, a};
            break;
        default:
            c = (Color){255, 255, 255, a};
            break;
        }

        int fs = 56 + (int)(8 * pulse);
        int tw = MeasureText(hype, fs);
        DrawText(hype, FIELD_W / 2 - tw / 2,
                 top + FIELD_H / 2 + 100, fs, c);
    }
#else
    (void)s;
#endif
}

/* ------- stats screen ------- */

void render_stats_screen(const MatchStats *ms, const Records *rec, int winner)
{
    DrawRectangleGradientV(0, 0, FIELD_W, FIELD_H + 120,
                           (Color){10, 25, 45, 255},
                           (Color){3, 8, 18, 255});

    const char *title = (winner == 1) ? "PLAYER 1 WINS!" : "PLAYER 2 WINS!";
    Color title_color = (winner == 1) ? BLUE : RED;
    int tw = MeasureText(title, 40);
    DrawText(title, FIELD_W / 2 - tw / 2, 40, 40, title_color);

    char buf[64];
    snprintf(buf, sizeof buf, "%d   -   %d", ms->goals_p1, ms->goals_p2);
    int sw = MeasureText(buf, 48);
    DrawText(buf, FIELD_W / 2 - sw / 2, 95, 48, RAYWHITE);

    int y = 170;
    DrawText("P1", FIELD_W / 4 - 15, y, 24, BLUE);
    DrawText("P2", 3 * FIELD_W / 4 - 15, y, 24, RED);
    y += 35;

    DrawLine(40, y, FIELD_W - 40, y, (Color){80, 120, 160, 200});
    y += 15;

#define ROW(label, v1, v2)                                      \
    do                                                          \
    {                                                           \
        DrawText(label, 50, y, 18, LIGHTGRAY);                  \
        char b[32];                                             \
        snprintf(b, sizeof b, "%s", v1);                        \
        int w1 = MeasureText(b, 22);                            \
        DrawText(b, FIELD_W / 4 - w1 / 2, y, 22, RAYWHITE);     \
        snprintf(b, sizeof b, "%s", v2);                        \
        int w2 = MeasureText(b, 22);                            \
        DrawText(b, 3 * FIELD_W / 4 - w2 / 2, y, 22, RAYWHITE); \
        y += 32;                                                \
    } while (0)

    char v1[32], v2[32];

    snprintf(v1, sizeof v1, "%d", ms->p1_hits);
    snprintf(v2, sizeof v2, "%d", ms->p2_hits);
    ROW("Hits", v1, v2);

    snprintf(v1, sizeof v1, "%d", ms->goals_p1);
    snprintf(v2, sizeof v2, "%d", ms->goals_p2);
    ROW("Goals", v1, v2);

    snprintf(v1, sizeof v1, "%d", ms->goals_conceded_p1);
    snprintf(v2, sizeof v2, "%d", ms->goals_conceded_p2);
    ROW("Conceded", v1, v2);

    snprintf(v1, sizeof v1, "%d", ms->p1_powerups);
    snprintf(v2, sizeof v2, "%d", ms->p2_powerups);
    ROW("Power-ups", v1, v2);

    y += 10;
    DrawLine(40, y, FIELD_W - 40, y, (Color){80, 120, 160, 200});
    y += 20;

    snprintf(buf, sizeof buf, "Longest Rally:  %d", ms->best_rally);
    int bw = MeasureText(buf, 22);
    DrawText(buf, FIELD_W / 2 - bw / 2, y, 22, YELLOW);
    y += 30;

    snprintf(buf, sizeof buf, "Top Puck Speed:  %.0f", ms->top_puck_speed);
    bw = MeasureText(buf, 22);
    DrawText(buf, FIELD_W / 2 - bw / 2, y, 22, (Color){200, 220, 240, 255});
    y += 30;

    int mins = (int)ms->match_time / 60;
    int secs = ((int)ms->match_time) % 60;
    snprintf(buf, sizeof buf, "Match Time:  %d:%02d", mins, secs);
    bw = MeasureText(buf, 22);
    DrawText(buf, FIELD_W / 2 - bw / 2, y, 22, (Color){200, 220, 240, 255});
    y += 35;

    DrawLine(40, y, FIELD_W - 40, y, (Color){80, 120, 160, 200});
    y += 15;

    DrawText("ALL-TIME RECORDS", 50, y, 16, GRAY);
    y += 24;

    snprintf(buf, sizeof buf, "Best Rally: %d    Fastest: %.0f    Matches: %d",
             rec->best_rally_ever, rec->fastest_puck_ever, rec->matches_played);
    bw = MeasureText(buf, 16);
    DrawText(buf, FIELD_W / 2 - bw / 2, y, 16, (Color){160, 200, 220, 255});
    y += 30;

    DrawText("Press ENTER for menu, R for rematch",
             FIELD_W / 2 - MeasureText("Press ENTER for menu, R for rematch", 20) / 2,
             FIELD_H + 40, 20, LIGHTGRAY);

#undef ROW
}