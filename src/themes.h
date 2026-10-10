#ifndef THEMES_H
#define THEMES_H

#include "raylib.h"

typedef enum
{
    THEME_NEON,
    THEME_RETRO,
    THEME_DARK,
    THEME_FIRE,
    THEME_COUNT
} ThemeId;

typedef struct
{
    Color field_top;
    Color field_bot;
    Color center_line;
    Color p1_color;
    Color p2_color;
    Color puck_color;
    Color hud_bg;
    Color hud_line;
    Color accent;
} Theme;

void themes_init(void);
const Theme *themes_get(ThemeId id);
const char *themes_name(ThemeId id);
void themes_next(void);
ThemeId themes_current(void);

#endif