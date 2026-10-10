#ifndef RENDER_H
#define RENDER_H

#include "game.h"
#include "stats.h"
#include "heatmap.h"
#include "tournament.h"
#include "coach.h"
#include "replay.h"

void render_init(void);
void render_field(const GameState *g);
void render_menu(int selected);
void render_gameover(const GameState *g, int winner);
void render_waiting_screen(const char *msg);
void render_solo_difficulty(int hover);
void render_combo_overlay(const MatchStats *s);
void render_stats_screen(const MatchStats *ms, const Records *rec, int winner);
void render_time_attack_hud(float time_left, int score);
void render_time_attack_results(int final_score, int best_score);
void render_tournament_hud(const Tournament *t);
void render_coach_screen(const CoachReport *rep, int winner);
void render_replay_banner(bool is_replay, int frames_left);

#endif