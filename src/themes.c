#include "themes.h"
#include "features.h"
#include <string.h>

static Theme s_themes[THEME_COUNT];
static ThemeId s_current = THEME_NEON;
static bool s_init = false;

static void setup_neon(void)
{
    s_themes[THEME_NEON] = (Theme){
        .field_top = (Color){12, 70, 100, 255},
        .field_bot = (Color){6, 40, 60, 255},
        .center_line = (Color){200, 220, 240, 150},
        .p1_color = BLUE,
        .p2_color = RED,
        .puck_color = YELLOW,
        .hud_bg = (Color){15, 20, 30, 255},
        .hud_line = (Color){80, 120, 160, 200},
        .accent = (Color){120, 200, 255, 255},
    };
}

static void setup_retro(void)
{
    s_themes[THEME_RETRO] = (Theme){
        .field_top = (Color){20, 15, 40, 255},
        .field_bot = (Color){10, 5, 25, 255},
        .center_line = (Color){255, 200, 100, 180},
        .p1_color = (Color){80, 220, 255, 255},
        .p2_color = (Color){255, 100, 180, 255},
        .puck_color = (Color){255, 240, 120, 255},
        .hud_bg = (Color){10, 8, 20, 255},
        .hud_line = (Color){255, 200, 100, 180},
        .accent = (Color){255, 200, 100, 255},
    };
}

static void setup_dark(void)
{
    s_themes[THEME_DARK] = (Theme){
        .field_top = (Color){30, 30, 35, 255},
        .field_bot = (Color){15, 15, 20, 255},
        .center_line = (Color){80, 80, 90, 200},
        .p1_color = (Color){200, 200, 220, 255},
        .p2_color = (Color){80, 80, 100, 255},
        .puck_color = (Color){240, 240, 250, 255},
        .hud_bg = (Color){10, 10, 12, 255},
        .hud_line = (Color){60, 60, 70, 200},
        .accent = (Color){200, 200, 220, 255},
    };
}

static void setup_fire(void)
{
    s_themes[THEME_FIRE] = (Theme){
        .field_top = (Color){80, 20, 10, 255},
        .field_bot = (Color){40, 5, 5, 255},
        .center_line = (Color){255, 180, 80, 200},
        .p1_color = (Color){255, 220, 80, 255},
        .p2_color = (Color){220, 50, 50, 255},
        .puck_color = (Color){255, 255, 200, 255},
        .hud_bg = (Color){25, 8, 5, 255},
        .hud_line = (Color){255, 120, 60, 200},
        .accent = (Color){255, 180, 80, 255},
    };
}

void themes_init(void)
{
    if (s_init)
        return;
    setup_neon();
    setup_retro();
    setup_dark();
    setup_fire();
    s_init = true;
}

const Theme *themes_get(ThemeId id)
{
    themes_init();
    if (id < 0 || id >= THEME_COUNT)
        id = THEME_NEON;
    return &s_themes[id];
}

const char *themes_name(ThemeId id)
{
    switch (id)
    {
    case THEME_NEON:
        return "Neon";
    case THEME_RETRO:
        return "Retro";
    case THEME_DARK:
        return "Dark";
    case THEME_FIRE:
        return "Fire";
    default:
        return "?";
    }
}

void themes_next(void)
{
    s_current = (ThemeId)((s_current + 1) % THEME_COUNT);
}

ThemeId themes_current(void)
{
    return s_current;
}

void themes_set_by_index(int idx)
{
    themes_init();
    if (idx < 0 || idx >= THEME_COUNT)
        idx = 0;
    s_current = (ThemeId)idx;
}