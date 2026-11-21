#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <stdbool.h>

/* --- Design tokens --- */
#define UI_BG_TOP (Color){10, 20, 35, 255}
#define UI_BG_BOT (Color){3, 8, 18, 255}
#define UI_PANEL (Color){20, 35, 55, 230}
#define UI_PANEL_LINE (Color){70, 110, 160, 200}
#define UI_ACCENT (Color){120, 200, 255, 255}
#define UI_ACCENT_HOT (Color){255, 220, 100, 255}
#define UI_TEXT (Color){220, 230, 245, 255}
#define UI_TEXT_DIM (Color){140, 160, 185, 255}
#define UI_TEXT_DARK (Color){100, 120, 140, 255}
#define UI_SUCCESS (Color){100, 255, 140, 255}
#define UI_DANGER (Color){255, 100, 100, 255}

/* --- Layout --- */
void ui_background(void);
void ui_title(const char *text, float pulse);
void ui_subtitle(const char *text);

/* --- Panels / cards --- */
void ui_panel(Rectangle r);
void ui_badge(const char *text, int x, int y, Color col);

/* --- Menu list (auto-spacing, animated cursor) --- */
int ui_menu(const char *const *items, int count, int selected,
            int center_x, int start_y, int spacing);

/* --- Button --- */
bool ui_button(Rectangle r, const char *text, bool hovered);

/* --- Hints --- */
void ui_hint(const char *text);

/* --- Toast notification --- */
void ui_toast(const char *text, float *timer);

/* Universal back button. Returns true if ESC or click on button. */
void ui_back_button_draw(void); /* draw only */
bool ui_back_button_input(void);

#endif