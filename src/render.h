#ifndef RENDER_H
#define RENDER_H

#include "game.h"

void render_init(void);
void render_field(const GameState *g);
void render_menu(int selected);
void render_gameover(const GameState *g, int winner);
void render_waiting_screen(const char *msg);

#endif