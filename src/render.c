#include "render.h"
#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

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

    /* screen shake offset */
    int shx = 0, shy = 0;
    if (g->shake > 0.1f)
    {
        shx = (rand() % 2 ? 1 : -1) * (int)g->shake;
        shy = (rand() % 2 ? 1 : -1) * (int)g->shake;
    }

    /* field gradient background */
    DrawRectangleGradientV(0, top, FIELD_W, FIELD_H,
                           (Color){12, 70, 100, 255},
                           (Color){6, 40, 60, 255});

    /* outer rim */
    DrawRectangleLines(0, top, FIELD_W, FIELD_H, (Color){120, 180, 220, 100});

    /* center line + circle */
    DrawLine(0, top + FIELD_H / 2, FIELD_W, top + FIELD_H / 2,
             (Color){200, 220, 240, 150});
    DrawCircleLines(FIELD_W / 2, top + FIELD_H / 2, 70,
                    (Color){200, 220, 240, 130});
    DrawCircleLines(FIELD_W / 2, top + FIELD_H / 2, 40,
                    (Color){200, 220, 240, 90});
    DrawCircle(FIELD_W / 2, top + FIELD_H / 2, 5,
               (Color){200, 220, 240, 200});

    /* goals */
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

    /* goal flash wash */
    if (g->goal_flash > 0)
    {
        Color wash = (g->last_scorer == 1)
                         ? (Color){255, 60, 60, (unsigned char)(g->goal_flash * 60)}
                         : (Color){60, 120, 255, (unsigned char)(g->goal_flash * 60)};
        DrawRectangle(0, top, FIELD_W, FIELD_H, wash);
    }

    /* paddles with rings */
    /* P1 — blue */
    DrawCircle((int)g->p1.x + shx, (int)(g->p1.y + top) + shy,
               PADDLE_R + 4, (Color){40, 90, 200, 90});
    DrawCircle((int)g->p1.x + shx, (int)(g->p1.y + top) + shy,
               PADDLE_R, BLUE);
    DrawCircleLines((int)g->p1.x + shx, (int)(g->p1.y + top) + shy,
                    PADDLE_R, RAYWHITE);
    DrawCircle((int)g->p1.x + shx, (int)(g->p1.y + top) + shy,
               5, RAYWHITE);

    /* P2 — red */
    DrawCircle((int)g->p2.x + shx, (int)(g->p2.y + top) + shy,
               PADDLE_R + 4, (Color){200, 50, 50, 90});
    DrawCircle((int)g->p2.x + shx, (int)(g->p2.y + top) + shy,
               PADDLE_R, RED);
    DrawCircleLines((int)g->p2.x + shx, (int)(g->p2.y + top) + shy,
                    PADDLE_R, RAYWHITE);
    DrawCircle((int)g->p2.x + shx, (int)(g->p2.y + top) + shy,
               5, RAYWHITE);

    /* puck with glow */
    DrawCircle((int)g->puck.x + shx, (int)(g->puck.y + top) + shy,
               PUCK_R + 6, (Color){255, 220, 100, 60});
    DrawCircle((int)g->puck.x + shx, (int)(g->puck.y + top) + shy,
               PUCK_R, YELLOW);
    DrawCircleLines((int)g->puck.x + shx, (int)(g->puck.y + top) + shy,
                    PUCK_R, (Color){120, 90, 0, 255});
    DrawCircle((int)g->puck.x + shx, (int)(g->puck.y + top) + shy,
               4, (Color){255, 240, 180, 255});

    /* ---------- HUD ---------- */
    /* score panel background */
    DrawRectangle(0, 0, FIELD_W, 100, (Color){15, 20, 30, 255});
    DrawLine(0, 100, FIELD_W, 100, (Color){80, 120, 160, 200});

    /* scores */
    char buf[16];
    snprintf(buf, sizeof buf, "%d", g->score1);
    int s1w = MeasureText(buf, 64);
    DrawText(buf, FIELD_W / 4 - s1w / 2, 12, 64, BLUE);

    snprintf(buf, sizeof buf, "%d", g->score2);
    int s2w = MeasureText(buf, 64);
    DrawText(buf, 3 * FIELD_W / 4 - s2w / 2, 12, 64, RED);

    /* labels */
    DrawText("P1", FIELD_W / 4 - 14, 78, 16, (Color){140, 170, 210, 255});
    DrawText("P2", 3 * FIELD_W / 4 - 14, 78, 16, (Color){210, 140, 140, 255});

    /* first to 7 */
    DrawText(TextFormat("FIRST TO %d", WIN_SCORE),
             FIELD_W / 2 - MeasureText(TextFormat("FIRST TO %d", WIN_SCORE), 16) / 2,
             42, 16, (Color){160, 180, 200, 200});

    /* ---------- countdown ---------- */
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
        /* pulsing alpha */
        float pulse = 0.6f + 0.4f * sinf(GetTime() * 8.0f);
        c.a = (unsigned char)(200 * pulse);

        DrawText(txt, FIELD_W / 2 - tw / 2, top + FIELD_H / 2 - 60, 120, c);
    }
}

/* ------- menu ------- */

void render_menu(int selected)
{
    /* gradient bg */
    DrawRectangleGradientV(0, 0, FIELD_W, FIELD_H + 120,
                           (Color){10, 25, 45, 255},
                           (Color){3, 8, 18, 255});

    /* title */
    const char *title = "AIR HOCKEY";
    int tw = MeasureText(title, 56);
    DrawText(title, FIELD_W / 2 - tw / 2, 70, 56, RAYWHITE);
    DrawText("C / SEA Project", FIELD_W / 2 - 70, 130, 16, GRAY);

    const char *items[4] = {
        "Local 2-Player",
        "Host LAN Game",
        "Join LAN Game",
        "Quit"};
    for (int i = 0; i < 4; i++)
    {
        Color c = (i == selected) ? YELLOW : LIGHTGRAY;
        int iw = MeasureText(items[i], 32);
        int x = FIELD_W / 2 - iw / 2;
        int y = 240 + i * 60;
        DrawText(items[i], x, y, 32, c);
        if (i == selected)
        {
            DrawText(">", x - 30, y, 32, YELLOW);
            DrawRectangle(x - 34, y + 36, iw + 60, 2, YELLOW);
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

    /* pulse dot */
    float t = GetTime();
    int r = 6 + (int)(4 * (0.5f + 0.5f * sinf(t * 4)));
    DrawCircle(30, 358, r, GREEN);
}