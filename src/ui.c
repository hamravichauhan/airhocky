#include "ui.h"
#include <string.h>
#include <math.h>
#include <stdio.h>

/* ------------ Background ------------ */

void ui_background(void)
{
    /* vertical gradient */
    DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(),
                           UI_BG_TOP, UI_BG_BOT);

    /* faint grid for tech feel */
    for (int x = 0; x < GetScreenWidth(); x += 60)
    {
        DrawLine(x, 0, x, GetScreenHeight(),
                 (Color){40, 60, 90, 30});
    }
    for (int y = 0; y < GetScreenHeight(); y += 60)
    {
        DrawLine(0, y, GetScreenWidth(), y,
                 (Color){40, 60, 90, 30});
    }

    /* subtle vignette */
    DrawRectangleGradientV(0, 0, GetScreenWidth(), 80,
                           (Color){0, 0, 0, 80}, (Color){0, 0, 0, 0});
    DrawRectangleGradientV(0, GetScreenHeight() - 80,
                           GetScreenWidth(), 80,
                           (Color){0, 0, 0, 0}, (Color){0, 0, 0, 80});
}

/* ------------ Titles ------------ */

void ui_title(const char *text, float pulse)
{
    int fs = 48;
    int tw = MeasureText(text, fs);
    int x = GetScreenWidth() / 2 - tw / 2;
    int y = 40;

    /* pulse glow */
    if (pulse > 0)
    {
        unsigned char a = (unsigned char)(80 * pulse);
        DrawText(text, x + 2, y + 2, fs, (Color){80, 180, 255, a});
        DrawText(text, x - 2, y - 2, fs, (Color){80, 180, 255, a});
    }

    DrawText(text, x, y, fs, UI_TEXT);

    /* underline */
    int line_w = tw + 40;
    int lx = GetScreenWidth() / 2 - line_w / 2;
    int ly = y + fs + 12;
    DrawRectangle(lx, ly, line_w, 2, UI_ACCENT);
}

void ui_subtitle(const char *text)
{
    int fs = 16;
    int tw = MeasureText(text, fs);
    DrawText(text, GetScreenWidth() / 2 - tw / 2, 130, fs, UI_TEXT_DIM);
}

/* ------------ Panels ------------ */

void ui_panel(Rectangle r)
{
    /* shadow */
    DrawRectangle((int)r.x + 4, (int)r.y + 4,
                  (int)r.width, (int)r.height,
                  (Color){0, 0, 0, 120});

    /* body */
    DrawRectangle((int)r.x, (int)r.y,
                  (int)r.width, (int)r.height, UI_PANEL);

    /* border */
    DrawRectangleLinesEx(r, 1.5f, UI_PANEL_LINE);

    /* top accent line */
    DrawRectangle((int)r.x, (int)r.y, (int)r.width, 3, UI_ACCENT);
}

void ui_badge(const char *text, int x, int y, Color col)
{
    int fs = 14;
    int tw = MeasureText(text, fs);
    int pad = 8;
    DrawRectangle(x, y, tw + pad * 2, fs + 8, col);
    DrawText(text, x + pad, y + 4, fs, (Color){20, 30, 45, 255});
}

/* ------------ Menu ------------ */

int ui_menu(const char *const *items, int count, int selected,
            int center_x, int start_y, int spacing)
{
    int mouse_hover = -1;

    for (int i = 0; i < count; i++)
    {
        Color c = (i == selected) ? UI_ACCENT_HOT : UI_TEXT;
        int fs = (i == selected) ? 32 : 26;
        int tw = MeasureText(items[i], fs);
        int x = center_x - tw / 2;
        int y = start_y + i * spacing;

        /* hover highlight */
        Rectangle hit = {x - 60, y - 6, tw + 120, fs + 12};
        bool hovered = CheckCollisionPointRec(GetMousePosition(), hit);
        if (hovered)
            mouse_hover = i;

        if (i == selected)
        {
            /* pulse scale */
            float pulse = 1.0f + 0.03f * sinf(GetTime() * 6.0f);
            int px = (int)(fs * pulse);

            /* glow behind */
            DrawRectangle(x - 40, y - 4, tw + 80, px + 8,
                          (Color){255, 220, 100, 30});

            /* cursor arrow */
            float ax = x - 40 + 6 * sinf(GetTime() * 4.0f);
            DrawTriangle(
                (Vector2){ax, y + px / 2.0f - 8},
                (Vector2){ax, y + px / 2.0f + 8},
                (Vector2){ax + 14, y + px / 2.0f},
                UI_ACCENT_HOT);

            DrawText(items[i], x, y, px, c);

            /* underline */
            DrawRectangle(x, y + px + 4, tw, 2, UI_ACCENT_HOT);
        }
        else
        {
            DrawText(items[i], x, y, fs, c);
            if (hovered)
            {
                DrawRectangle(x - 10, y + fs + 2, tw + 20, 1,
                              (Color){120, 200, 255, 120});
            }
        }
    }

    return mouse_hover;
}

/* ------------ Button ------------ */

bool ui_button(Rectangle r, const char *text, bool hovered)
{
    bool clicked = false;
    Vector2 m = GetMousePosition();
    if (CheckCollisionPointRec(m, r))
    {
        hovered = true;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            clicked = true;
    }

    Color bg = hovered ? (Color){50, 80, 120, 255} : UI_PANEL;
    Color border = hovered ? UI_ACCENT_HOT : UI_PANEL_LINE;
    Color text_c = hovered ? UI_ACCENT_HOT : UI_TEXT;

    DrawRectangleRec(r, bg);
    DrawRectangleLinesEx(r, 1.5f, border);

    int fs = 20;
    if (r.height < 36)
        fs = 15;
    while (fs > 10 && MeasureText(text, fs) > (int)r.width - 12)
        fs--;

    int tw = MeasureText(text, fs);
    DrawText(text,
             (int)(r.x + r.width / 2 - tw / 2),
             (int)(r.y + r.height / 2 - fs / 2),
             fs, text_c);

    return clicked;
}

/* ------------ Hints ------------ */

void ui_hint(const char *text)
{
    int fs = 16;
    int tw = MeasureText(text, fs);
    DrawText(text, GetScreenWidth() / 2 - tw / 2,
             GetScreenHeight() - 40, fs, UI_TEXT_DARK);
}

/* ------------ Toast ------------ */

void ui_toast(const char *text, float *timer)
{
    if (*timer <= 0)
        return;

    *timer -= GetFrameTime();

    float elapsed = 1.5f - *timer; /* assumes 1.5s toast */
    float alpha = 1.0f;
    if (elapsed < 0.2f)
        alpha = elapsed / 0.2f;
    if (*timer < 0.4f)
        alpha = *timer / 0.4f;
    if (alpha > 1.0f)
        alpha = 1.0f;
    if (alpha < 0.0f)
        alpha = 0.0f;

    int fs = 20;
    int tw = MeasureText(text, fs);
    int pad = 16;
    int w = tw + pad * 2;
    int h = fs + pad;
    int x = GetScreenWidth() / 2 - w / 2;
    int y = GetScreenHeight() - 100;

    unsigned char a = (unsigned char)(220 * alpha);
    DrawRectangle(x, y, w, h, (Color){20, 40, 60, a});
    DrawRectangleLines(x, y, w, h, (Color){120, 200, 255, a});
    DrawText(text, x + pad, y + pad / 2, fs,
             (Color){220, 230, 245, (unsigned char)(255 * alpha)});
}

bool ui_back_button(void)
{
    Rectangle btn = {20, 20, 110, 40};

    Vector2 m = GetMousePosition();
    bool hovered = CheckCollisionPointRec(m, btn);
    bool clicked = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool pressed_esc = IsKeyPressed(KEY_ESCAPE);

    Color bg = hovered ? (Color){50, 80, 120, 255} : UI_PANEL;
    Color border = hovered ? UI_ACCENT_HOT : UI_PANEL_LINE;
    Color text_c = hovered ? UI_ACCENT_HOT : UI_TEXT;

    DrawRectangleRec(btn, bg);
    DrawRectangleLinesEx(btn, 1.5f, border);

    /* left arrow */
    float ax = btn.x + 12;
    float ay = btn.y + btn.height / 2.0f;
    DrawTriangle(
        (Vector2){ax + 10, ay - 8},
        (Vector2){ax + 10, ay + 8},
        (Vector2){ax, ay},
        text_c);

    DrawText("BACK", (int)(btn.x + 30),
             (int)(btn.y + btn.height / 2.0f - 10),
             20, text_c);

    /* ESC hint to the right */
    DrawText("[ESC]", (int)(btn.x + btn.width + 10),
             (int)(btn.y + btn.height / 2.0f - 8),
             14, UI_TEXT_DARK);

    return clicked || pressed_esc;
}

void ui_back_button_draw(void)
{
    Rectangle btn = {20, 20, 110, 40};
    Vector2 m = GetMousePosition();
    bool hovered = CheckCollisionPointRec(m, btn);

    Color bg = hovered ? (Color){50, 80, 120, 255} : UI_PANEL;
    Color border = hovered ? UI_ACCENT_HOT : UI_PANEL_LINE;
    Color text_c = hovered ? UI_ACCENT_HOT : UI_TEXT;

    DrawRectangleRec(btn, bg);
    DrawRectangleLinesEx(btn, 1.5f, border);

    float ax = btn.x + 12;
    float ay = btn.y + btn.height / 2.0f;
    DrawTriangle(
        (Vector2){ax + 10, ay - 8},
        (Vector2){ax + 10, ay + 8},
        (Vector2){ax, ay},
        text_c);

    DrawText("BACK", (int)(btn.x + 30),
             (int)(btn.y + btn.height / 2.0f - 10),
             20, text_c);

    DrawText("[ESC]", (int)(btn.x + btn.width + 10),
             (int)(btn.y + btn.height / 2.0f - 8),
             14, UI_TEXT_DARK);
}

bool ui_back_button_input(void)
{
    Rectangle btn = {20, 20, 110, 40};
    Vector2 m = GetMousePosition();
    bool hovered = CheckCollisionPointRec(m, btn);
    bool clicked = hovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool pressed_esc = IsKeyPressed(KEY_ESCAPE);
    return clicked || pressed_esc;
}