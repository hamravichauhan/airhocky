#ifndef RENDER_H
#define RENDER_H

#include "game.h"
#include "stats.h"

void render_init(void);
void render_field(const GameState *g);
void render_menu(int selected);
void render_gameover(const GameState *g, int winner);
void render_waiting_screen(const char *msg);
void render_solo_difficulty(int hover);
void render_combo_overlay(const MatchStats *s);
void render_stats_screen(const MatchStats *ms, const Records *rec, int winner);

#endif