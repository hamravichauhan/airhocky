#ifndef SETTINGS_UI_H
#define SETTINGS_UI_H

#include "settings.h"
#include <stdbool.h>

bool settings_ui_update(GameSettings *s);
void settings_ui_draw(const GameSettings *s);

#endif