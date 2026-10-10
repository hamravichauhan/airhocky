#include "raylib.h"
#include "audio.h"
#include "game.h"
#include "render.h"
#include "input.h"
#include "net.h"
#include "ai.h"
#include "particles.h"
#include "powerup.h"
#include "stats.h"
#include "features.h"
#include "heatmap.h"
#include "themes.h"
#include "trajectory.h"
#include "commentary.h"
#include "dynaudio.h"
#include "tournament.h"
#include "coach.h"
#include "replay.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

typedef enum
{
    SCREEN_MENU,
    SCREEN_SOLO_DIFF,
    SCREEN_HOST_WAIT,
    SCREEN_JOIN_INPUT,
    SCREEN_PLAYING,
    SCREEN_GAMEOVER,
    SCREEN_STATS,
    SCREEN_TIME_ATTACK,
    SCREEN_TIME_ATTACK_OVER,
    SCREEN_TOURNAMENT_DIFF,
    SCREEN_COACH,
    SCREEN_REPLAY_LOADED
} Screen;

int main(void)
{
    InitWindow(FIELD_W, FIELD_H + 120, "Air Hockey");
    SetTargetFPS(60);
    srand((unsigned)GetTime() * 1000);

    InitAudioDevice();
    audio_init();
    particles_init();
    themes_init();

    Screen screen = SCREEN_MENU;
    int menu_sel = 0;
    const int menu_n = 9;

    GameState game;
    memset(&game, 0, sizeof(game));

    AIController ai;
    memset(&ai, 0, sizeof(ai));

    PowerUpState powerups;
    powerup_init(&powerups);

    MatchStats stats;
    stats_init(&stats);

    Records records;
    records_load(&records);

    HeatMap heat;
    heatmap_init(&heat);
    heat.visible = true;

    Commentary commentary;
    commentary_init(&commentary);

    DynamicAudio dynaudio;
    dynaudio_init(&dynaudio);

    Tournament tour;
    memset(&tour, 0, sizeof(tour));
    bool in_tournament = false;

    CoachReport coach;
    coach_clear(&coach);

    Replay replay;
    replay_init(&replay);

    GameState game_before_replay;
    memset(&game_before_replay, 0, sizeof(game_before_replay));
    bool had_replay = false;

    Replay loaded_replay;
    replay_init(&loaded_replay);

    bool demo_mode = false;

    float ta_time_left = 60.0f;
    int ta_score = 0;
    int ta_best = 0;
    bool ta_active = false;

    char join_ip[64] = "127.0.0.1";
    int ip_len = 9;

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        if (dt > 0.05f)
            dt = 0.05f;

        if (screen == SCREEN_PLAYING || screen == SCREEN_TIME_ATTACK)
        {
            if (IsKeyPressed(KEY_TAB))
                heat.visible = !heat.visible;
            if (IsKeyPressed(KEY_T))
                themes_next();
        }

        switch (screen)
        {
        case SCREEN_MENU:
            if (IsKeyPressed(KEY_DOWN))
                menu_sel = (menu_sel + 1) % menu_n;
            if (IsKeyPressed(KEY_UP))
                menu_sel = (menu_sel + menu_n - 1) % menu_n;
            if (IsKeyPressed(KEY_ENTER))
            {
                in_tournament = false;
                switch (menu_sel)
                {
                case 0: /* Solo */
                    screen = SCREEN_SOLO_DIFF;
                    break;
                case 1: /* Local */
                    game_init(&game, MODE_LOCAL);
                    powerup_init(&powerups);
                    stats_init(&stats);
                    particles_clear();
                    heatmap_init(&heat);
                    heat.visible = true;
                    replay_init(&replay);
                    demo_mode = false;
                    screen = SCREEN_PLAYING;
                    break;
                case 2: /* Host */
                    if (net_host_start())
                        screen = SCREEN_HOST_WAIT;
                    else
                        TraceLog(LOG_ERROR, "Failed to host");
                    break;
                case 3: /* Join */
                    screen = SCREEN_JOIN_INPUT;
                    break;
                case 4: /* Time Attack */
                    game_init(&game, MODE_SOLO);
                    ai_init(&ai, AI_MEDIUM);
                    powerup_init(&powerups);
                    stats_init(&stats);
                    particles_clear();
                    heatmap_init(&heat);
                    heat.visible = true;
                    replay_init(&replay);
                    demo_mode = false;
                    ta_time_left = 60.0f;
                    ta_score = 0;
                    ta_active = true;
                    screen = SCREEN_TIME_ATTACK;
                    break;
                case 5: /* Tournament */
                    screen = SCREEN_TOURNAMENT_DIFF;
                    break;
                case 6: /* AI vs AI Demo */
                    game_init(&game, MODE_SOLO);
                    ai_init(&ai, AI_MEDIUM);
                    powerup_init(&powerups);
                    stats_init(&stats);
                    particles_clear();
                    heatmap_init(&heat);
                    heat.visible = false;
                    replay_init(&replay);
                    demo_mode = true;
                    screen = SCREEN_PLAYING;
                    break;
                case 7:
                { /* Watch Replay */
                    replay_init(&loaded_replay);
                    if (replay_load(&loaded_replay.buf, "replay.replay"))
                    {
                        loaded_replay.active = true;
                        loaded_replay.play_start = 0;
                        loaded_replay.play_idx = 0;
                        loaded_replay.frame_timer = 0;

                        /* Reset game state so old positions don't show */
                        memset(&game, 0, sizeof(game));

                        screen = SCREEN_REPLAY_LOADED;
                        TraceLog(LOG_INFO, "Replay loaded");
                    }
                    else
                    {
                        TraceLog(LOG_WARNING, "No replay.replay found");
                    }
                    break;
                }
                case 8: /* Quit */
                    audio_close();
                    CloseAudioDevice();
                    CloseWindow();
                    return 0;
                }
            }
            break;

        case SCREEN_TOURNAMENT_DIFF:
            if (IsKeyPressed(KEY_ESCAPE))
                screen = SCREEN_MENU;
            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1))
            {
                tournament_init(&tour, TOUR_BO3);
                in_tournament = true;
                game_init(&game, MODE_LOCAL);
                powerup_init(&powerups);
                stats_init(&stats);
                particles_clear();
                heatmap_init(&heat);
                heat.visible = true;
                replay_init(&replay);
                demo_mode = false;
                screen = SCREEN_PLAYING;
            }
            if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2))
            {
                tournament_init(&tour, TOUR_BO5);
                in_tournament = true;
                game_init(&game, MODE_LOCAL);
                powerup_init(&powerups);
                stats_init(&stats);
                particles_clear();
                heatmap_init(&heat);
                heat.visible = true;
                replay_init(&replay);
                demo_mode = false;
                screen = SCREEN_PLAYING;
            }
            break;

        case SCREEN_SOLO_DIFF:
            if (IsKeyPressed(KEY_ESCAPE))
                screen = SCREEN_MENU;
            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1))
            {
                game_init(&game, MODE_SOLO);
                ai_init(&ai, AI_EASY);
                powerup_init(&powerups);
                stats_init(&stats);
                particles_clear();
                heatmap_init(&heat);
                heat.visible = true;
                replay_init(&replay);
                demo_mode = false;
                screen = SCREEN_PLAYING;
            }
            if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2))
            {
                game_init(&game, MODE_SOLO);
                ai_init(&ai, AI_MEDIUM);
                powerup_init(&powerups);
                stats_init(&stats);
                particles_clear();
                heatmap_init(&heat);
                heat.visible = true;
                replay_init(&replay);
                demo_mode = false;
                screen = SCREEN_PLAYING;
            }
            if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3))
            {
                game_init(&game, MODE_SOLO);
                ai_init(&ai, AI_HARD);
                powerup_init(&powerups);
                stats_init(&stats);
                particles_clear();
                heatmap_init(&heat);
                heat.visible = true;
                replay_init(&replay);
                demo_mode = false;
                screen = SCREEN_PLAYING;
            }
            break;

        case SCREEN_HOST_WAIT:
        {
            float tx, ty;
            (void)net_poll_input(&tx, &ty);
            if (net_is_connected())
            {
                game_init(&game, MODE_HOST);
                powerup_init(&powerups);
                stats_init(&stats);
                particles_clear();
                heatmap_init(&heat);
                replay_init(&replay);
                demo_mode = false;
                screen = SCREEN_PLAYING;
            }
        }
            if (IsKeyPressed(KEY_ESCAPE))
            {
                net_close();
                screen = SCREEN_MENU;
            }
            break;

        case SCREEN_JOIN_INPUT:
        {
            int key = GetCharPressed();
            while (key > 0)
            {
                if (key >= 32 && key < 127 && ip_len < 63)
                {
                    join_ip[ip_len++] = (char)key;
                    join_ip[ip_len] = '\0';
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && ip_len > 0)
                join_ip[--ip_len] = '\0';

            if (IsKeyPressed(KEY_ENTER))
            {
                if (net_client_connect(join_ip))
                {
                    game_init(&game, MODE_CLIENT);
                    powerup_init(&powerups);
                    stats_init(&stats);
                    particles_clear();
                    heatmap_init(&heat);
                    replay_init(&replay);
                    demo_mode = false;
                    screen = SCREEN_PLAYING;
                }
                else
                {
                    TraceLog(LOG_ERROR, "Connect failed");
                }
            }
            if (IsKeyPressed(KEY_ESCAPE))
                screen = SCREEN_MENU;
            break;
        }

        case SCREEN_PLAYING:
        case SCREEN_TIME_ATTACK:
        {
            bool is_ta = (screen == SCREEN_TIME_ATTACK);

            if (demo_mode && IsKeyPressed(KEY_ESCAPE))
            {
                demo_mode = false;
                screen = SCREEN_MENU;
                menu_sel = 0;
                break;
            }

            /* ---------- REPLAY HANDLING ---------- */
            /* If a replay is running, don't simulate physics */
            if (replay_is_active(&replay))
            {
                replay_update(&replay, dt);

                if (!replay_is_active(&replay))
                {
                    /* Replay just ended → restore saved state */
                    if (had_replay)
                    {
                        game = game_before_replay;
                        had_replay = false;
                    }
                }
                else
                {
                    /* Fetch current replay frame into game state */
                    ReplayFrame rf;
                    replay_get_current(&replay, &rf);
                    game.p1.x = rf.p1x;
                    game.p1.y = rf.p1y;
                    game.p2.x = rf.p2x;
                    game.p2.y = rf.p2y;
                    game.puck.x = rf.puck_x;
                    game.puck.y = rf.puck_y;
                    game.puck.vx = rf.puck_vx;
                    game.puck.vy = rf.puck_vy;
                    game.puck_spin = rf.spin;
                }

                break; /* skip rest of SCREEN_PLAYING update */
            }

            /* Trigger replay manually */
            if (IsKeyPressed(KEY_V))
            {
                /* Save current state so we can restore it after */
                game_before_replay = game;
                had_replay = true;

                replay_start(&replay);
                /* If replay didn't activate (not enough data), undo */
                if (!replay_is_active(&replay))
                {
                    had_replay = false;
                }
                break;
            }

            /* Save to disk */
            if (IsKeyPressed(KEY_F5))
            {
                if (replay_save(&replay.buf, "replay.replay"))
                {
                    TraceLog(LOG_INFO, "Replay saved to replay.replay");
                }
            }
            /* ---------- END REPLAY HANDLING ---------- */

            if (is_ta && ta_active)
            {
                ta_time_left -= dt;
                if (ta_time_left <= 0)
                {
                    ta_time_left = 0;
                    ta_active = false;
                    if (ta_score > ta_best)
                        ta_best = ta_score;
                    screen = SCREEN_TIME_ATTACK_OVER;
                    break;
                }
            }

            bool simulate_powerups =
                (game.mode == MODE_LOCAL ||
                 game.mode == MODE_SOLO ||
                 game.mode == MODE_HOST);

            powerup_update(&powerups, &game, dt, simulate_powerups ? 1 : 0);

            game.p1_radius_mult = powerup_paddle_radius(&powerups, 1) / PADDLE_R;
            game.p2_radius_mult = powerup_paddle_radius(&powerups, 2) / PADDLE_R;
            game.puck_speed_mult = powerup_puck_speed_scale(&powerups);
            game.p1_frozen = powerup_paddle_frozen(&powerups, 1);
            game.p2_frozen = powerup_paddle_frozen(&powerups, 2);

            game.p1_spin_bias = IsKeyDown(KEY_LEFT_SHIFT)
                                    ? ((game.p1.vx > 0) ? 1.0f : -1.0f)
                                    : 0.0f;
            if (game.mode == MODE_LOCAL)
            {
                game.p2_spin_bias = IsKeyDown(KEY_RIGHT_SHIFT)
                                        ? ((game.p2.vx > 0) ? 1.0f : -1.0f)
                                        : 0.0f;
            }
            else
            {
                game.p2_spin_bias = 0.0f;
            }

            if (game.mode == MODE_LOCAL)
            {
                float p1x, p1y, p2x, p2y;
                input_get_local_target(&game, 1, &p1x, &p1y);
                input_get_local_target(&game, 2, &p2x, &p2y);
                game_update(&game, dt, p1x, p1y, p2x, p2y);
            }
            else if (game.mode == MODE_SOLO)
            {
                float p1x, p1y, p2x, p2y;

                if (demo_mode)
                {
                    ai_update(&ai, &game, dt);
                    ai_get_target(&ai, &p2x, &p2y);

                    p1x = game.puck.x;
                    p1y = FIELD_H - 100.0f;
                    if (game.puck.y > FIELD_H / 2.0f &&
                        game.puck.y < FIELD_H - 60.0f)
                    {
                        p1y = game.puck.y - 30.0f;
                    }
                }
                else
                {
                    input_get_local_target(&game, 1, &p1x, &p1y);
                    ai_update(&ai, &game, dt);
                    ai_get_target(&ai, &p2x, &p2y);
                }

                game_update(&game, dt, p1x, p1y, p2x, p2y);
            }
            else if (game.mode == MODE_HOST)
            {
                float p1x, p1y;
                input_get_local_target(&game, 1, &p1x, &p1y);
                float p2x = game.p2.x, p2y = game.p2.y;
                float rx, ry;
                while (net_poll_input(&rx, &ry))
                {
                    p2x = rx;
                    p2y = ry;
                }
                game_update(&game, dt, p1x, p1y, p2x, p2y);
                net_send_state(&game);
                if (!net_is_connected())
                {
                    screen = SCREEN_MENU;
                    net_close();
                }
            }
            else if (game.mode == MODE_CLIENT)
            {
                float p1x, p1y;
                input_get_local_target(&game, 1, &p1x, &p1y);
                net_send_input(p1x, p1y);
                while (net_poll_state(&game))
                {
                }
                if (!net_is_connected())
                {
                    screen = SCREEN_MENU;
                    net_close();
                }
            }

            /* Record frame into replay buffer */
            replay_record(&replay, &game);

            stats_update(&stats, &game, dt);
            commentary_update(&commentary, dt);

            if (game.ev_hit)
            {
                int hitter = (game.puck.y < FIELD_H / 2.0f) ? 2 : 1;
                stats_on_hit(&stats, hitter);
                ai_record_player_hit(&ai, game.puck.x);

                if (game.ev_hit_intensity > 0.75f)
                    commentary_trigger(&commentary, CMT_POWER_SHOT);
                else if (game.puck.y < 200.0f && game.puck.vy > 100.0f)
                    commentary_trigger(&commentary, CMT_GREAT_SAVE);
            }
            if (game.ev_goal)
            {
                stats_on_goal(&stats, game.last_scorer);
                ai_record_player_goal(&ai, game.puck.x);
                if (is_ta && game.last_scorer == 1)
                    ta_score++;

                commentary_trigger(&commentary,
                                   game.last_scorer == 1 ? CMT_GOAL_P1 : CMT_GOAL_P2);
            }

            heatmap_record(&heat, game.puck.x, game.puck.y);
            heatmap_decay(&heat, dt);

            float intensity = stats.current_rally / 20.0f;
            if (intensity > 1.0f)
                intensity = 1.0f;
            dynaudio_set_intensity(&dynaudio, intensity);
            dynaudio_update(&dynaudio, dt);

            if (game.ev_hit)
                audio_play(SFX_HIT, game.ev_hit_intensity);
            if (game.ev_wall)
                audio_play(SFX_WALL, game.ev_wall_intensity);
            if (game.ev_goal)
                audio_play(SFX_GOAL, 1.0f);
            if (game.ev_count_tick > 0)
                audio_play(SFX_COUNT, 1.0f);
            if (game.ev_go)
                audio_play(SFX_GO, 1.0f);

            if (stats.hype_timer > 1.79f && stats.hype != HYPE_NONE)
            {
                audio_play(SFX_HYPE, 1.0f);
                Vector2 mid = {FIELD_W / 2.0f, FIELD_H / 2.0f};
                particles_emit_ring(mid, (Color){255, 220, 100, 255});

                if (stats.hype == HYPE_GREAT)
                    commentary_trigger(&commentary, CMT_RALLY_10);
                if (stats.hype == HYPE_INSANE)
                    commentary_trigger(&commentary, CMT_RALLY_20);
                if (stats.hype == HYPE_LEGENDARY)
                    commentary_trigger(&commentary, CMT_INSANE);
            }

            if (commentary.timer > 0 &&
                !commentary.shown &&
                commentary.timer > commentary.duration - 0.1f)
            {
                audio_play(SFX_VOICE, dynaudio_get(&dynaudio));
                commentary.shown = true;
            }

            float puck_speed = sqrtf(game.puck.vx * game.puck.vx +
                                     game.puck.vy * game.puck.vy);
            if (puck_speed > 80.0f && game.reset_timer <= 0)
            {
                particles_trail_add((Vector2){game.puck.x, game.puck.y});
            }
            if (game.ev_hit && game.ev_hit_pos_valid)
            {
                Vector2 hp = {game.ev_hit_pos_x, game.ev_hit_pos_y};
                Vector2 hd = {game.ev_hit_dir_x, game.ev_hit_dir_y};
                particles_emit_spark(hp, hd, game.ev_hit_intensity);
            }
            if (game.ev_wall && game.ev_wall_pos_valid)
            {
                Vector2 wp = {game.ev_wall_pos_x, game.ev_wall_pos_y};
                particles_emit_dust(wp, 5);
            }
            if (game.ev_goal)
            {
                particles_trail_clear();
                Vector2 gp = {FIELD_W / 2.0f,
                              (game.last_scorer == 1) ? 40.0f : FIELD_H - 40.0f};
                Color theme = (game.last_scorer == 1) ? BLUE : RED;
                particles_emit_goal(gp, theme);
            }
            if (game.ev_count_tick > 0)
            {
                Vector2 center = {FIELD_W / 2.0f, FIELD_H / 2.0f};
                Color rc = (game.ev_count_tick == 1) ? YELLOW : RAYWHITE;
                particles_emit_ring(center, rc);
            }
            if (powerups.ev_pickup)
            {
                audio_play(SFX_GO, 1.0f);
                Vector2 pos;
                pos.x = FIELD_W / 2.0f;
                pos.y = (powerups.ev_pickup_player == 1) ? FIELD_H * 0.7f
                                                         : FIELD_H * 0.3f;
                Color c = (powerups.ev_pickup_player == 1)
                              ? (Color){100, 200, 255, 255}
                              : (Color){255, 100, 100, 255};
                particles_emit_ring(pos, c);
            }
            if (powerups.ev_tie)
            {
                audio_play(SFX_WALL, 0.6f);
                Vector2 mid = {FIELD_W / 2.0f, FIELD_H / 2.0f};
                particles_emit_ring(mid, GRAY);
            }

            if (!is_ta && !demo_mode && game_check_win(&game))
            {
                audio_play(SFX_WIN, 1.0f);
                stats.p1_powerups = powerups.p1_pickups;
                stats.p2_powerups = powerups.p2_pickups;
                records_check_and_update(&stats, &records);

                int winner = (game.score1 > game.score2) ? 1 : 2;

                if (in_tournament)
                {
                    tournament_record_match(&tour, winner);
                    if (tournament_is_over(&tour))
                    {
                        in_tournament = false;
                        screen = SCREEN_STATS;
                    }
                    else
                    {
                        game_init(&game, MODE_LOCAL);
                        powerup_init(&powerups);
                        stats_init(&stats);
                        particles_clear();
                        heatmap_init(&heat);
                        heat.visible = true;
                        replay_init(&replay);
                        screen = SCREEN_PLAYING;
                    }
                }
                else if (game.mode == MODE_SOLO)
                {
                    coach_analyze(&coach, &stats, winner == 1);
                    screen = SCREEN_COACH;
                }
                else
                {
                    screen = SCREEN_STATS;
                }
            }
            break;
        }

        case SCREEN_REPLAY_LOADED:
        {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER))
            {

                screen = SCREEN_MENU;
                menu_sel = 0;
                break;
            }

            if (loaded_replay.active)
            {
                loaded_replay.frame_timer += dt;
                while (loaded_replay.frame_timer >= loaded_replay.frame_period)
                {
                    loaded_replay.frame_timer -= loaded_replay.frame_period;
                    loaded_replay.play_idx++;
                    if (loaded_replay.play_idx >= loaded_replay.buf.count)
                    {
                        loaded_replay.active = false;
                        break;
                    }
                }

                if (loaded_replay.active)
                {
                    int idx = (loaded_replay.play_start + loaded_replay.play_idx) % REPLAY_FRAMES;
                    ReplayFrame *f = &loaded_replay.buf.frames[idx];
                    game.p1.x = f->p1x;
                    game.p1.y = f->p1y;
                    game.p2.x = f->p2x;
                    game.p2.y = f->p2y;
                    game.puck.x = f->puck_x;
                    game.puck.y = f->puck_y;
                    game.puck.vx = f->puck_vx;
                    game.puck.vy = f->puck_vy;
                    game.puck_spin = f->spin;
                }
            }
            break;
        }

        case SCREEN_COACH:
            if (IsKeyPressed(KEY_ENTER))
            {
                screen = SCREEN_MENU;
                menu_sel = 0;
            }
            if (IsKeyPressed(KEY_R))
            {
                game_init(&game, MODE_SOLO);
                ai_init(&ai, AI_MEDIUM);
                powerup_init(&powerups);
                stats_init(&stats);
                particles_clear();
                heatmap_init(&heat);
                replay_init(&replay);
                screen = SCREEN_PLAYING;
            }
            break;

        case SCREEN_TIME_ATTACK_OVER:
            if (IsKeyPressed(KEY_ENTER))
            {
                screen = SCREEN_MENU;
                menu_sel = 0;
            }
            if (IsKeyPressed(KEY_R))
            {
                game_init(&game, MODE_SOLO);
                ai_init(&ai, AI_MEDIUM);
                powerup_init(&powerups);
                stats_init(&stats);
                particles_clear();
                heatmap_init(&heat);
                heat.visible = true;
                replay_init(&replay);
                ta_time_left = 60.0f;
                ta_score = 0;
                ta_active = true;
                screen = SCREEN_TIME_ATTACK;
            }
            break;

        case SCREEN_STATS:
            if (IsKeyPressed(KEY_ENTER))
            {
                net_close();
                screen = SCREEN_MENU;
                menu_sel = 0;
            }
            if (IsKeyPressed(KEY_R))
            {
                net_close();
                if (game.mode == MODE_LOCAL)
                {
                    game_init(&game, MODE_LOCAL);
                    powerup_init(&powerups);
                    stats_init(&stats);
                    particles_clear();
                    heatmap_init(&heat);
                    replay_init(&replay);
                    screen = SCREEN_PLAYING;
                }
                else if (game.mode == MODE_SOLO)
                {
                    game_init(&game, MODE_SOLO);
                    powerup_init(&powerups);
                    stats_init(&stats);
                    particles_clear();
                    heatmap_init(&heat);
                    replay_init(&replay);
                    screen = SCREEN_PLAYING;
                }
                else
                {
                    screen = SCREEN_MENU;
                }
            }
            break;

        case SCREEN_GAMEOVER:
            if (IsKeyPressed(KEY_ENTER))
            {
                net_close();
                screen = SCREEN_MENU;
                menu_sel = 0;
            }
            break;
        }

        particles_update(dt);

        BeginDrawing();
        ClearBackground((Color){20, 25, 35, 255});

        switch (screen)
        {
        case SCREEN_MENU:
            render_menu(menu_sel);
            break;

        case SCREEN_SOLO_DIFF:
            render_solo_difficulty(menu_sel);
            break;

        case SCREEN_TOURNAMENT_DIFF:
            DrawRectangleGradientV(0, 0, FIELD_W, FIELD_H + 120,
                                   (Color){10, 25, 45, 255},
                                   (Color){3, 8, 18, 255});
            DrawText("TOURNAMENT FORMAT",
                     FIELD_W / 2 - MeasureText("TOURNAMENT FORMAT", 36) / 2,
                     140, 36, RAYWHITE);
            DrawText("1  -  Best of 3",
                     FIELD_W / 2 - MeasureText("1  -  Best of 3", 30) / 2,
                     260, 30, GREEN);
            DrawText("First to 2 match wins",
                     FIELD_W / 2 - MeasureText("First to 2 match wins", 18) / 2,
                     298, 18, GRAY);
            DrawText("2  -  Best of 5",
                     FIELD_W / 2 - MeasureText("2  -  Best of 5", 30) / 2,
                     380, 30, YELLOW);
            DrawText("First to 3 match wins",
                     FIELD_W / 2 - MeasureText("First to 3 match wins", 18) / 2,
                     418, 18, GRAY);
            DrawText("1/2 to choose   ESC = back",
                     FIELD_W / 2 - MeasureText("1/2 to choose   ESC = back", 20) / 2,
                     FIELD_H + 40, 20, LIGHTGRAY);
            break;

        case SCREEN_HOST_WAIT:
            render_waiting_screen("Waiting for client...  Share your IP. ESC = cancel");
            break;

        case SCREEN_JOIN_INPUT:
            DrawText("Enter host IP:", 40, 200, 28, RAYWHITE);
            DrawText(join_ip, 40, 250, 32, GREEN);
            DrawText("ENTER = connect   ESC = back", 40, 320, 20, LIGHTGRAY);
            break;

        case SCREEN_PLAYING:
            render_field(&game);
            if (heat.visible && !replay_is_active(&replay))
                heatmap_draw(&heat, 100);
            if (!replay_is_active(&replay))
                trajectory_draw(&game, 100, IsKeyDown(KEY_LEFT_CONTROL));
            particles_draw(100);
            if (!replay_is_active(&replay))
                powerup_draw(&powerups, 100);
            if (in_tournament)
                render_tournament_hud(&tour);
            if (!replay_is_active(&replay))
                render_combo_overlay(&stats);
            commentary_draw(&commentary, 100);
            render_replay_banner(replay_is_active(&replay), 0);

            if (demo_mode)
            {
                DrawText("DEMO MODE - Press ESC to stop",
                         FIELD_W / 2 - MeasureText("DEMO MODE - Press ESC to stop", 20) / 2,
                         FIELD_H + 30, 20, YELLOW);
            }
            else if (replay_is_active(&replay))
            {
                DrawText("V: replay   F5: save replay",
                         10, FIELD_H + 130, 16, GRAY);
            }
            else
            {
                DrawText(TextFormat("Theme: %s (T)", themes_name(themes_current())),
                         10, FIELD_H + 130, 16, GRAY);
                DrawText("TAB: heatmap  CTRL: trajectory  SHIFT: spin  V: replay  F5: save",
                         10, FIELD_H + 150, 14, DARKGRAY);
            }
            break;

        case SCREEN_TIME_ATTACK:
            render_field(&game);
            if (heat.visible && !replay_is_active(&replay))
                heatmap_draw(&heat, 100);
            particles_draw(100);
            if (!replay_is_active(&replay))
                powerup_draw(&powerups, 100);
            if (!replay_is_active(&replay))
                render_combo_overlay(&stats);
            commentary_draw(&commentary, 100);
            if (!replay_is_active(&replay))
                render_time_attack_hud(ta_time_left, ta_score);
            render_replay_banner(replay_is_active(&replay), 0);
            break;

        case SCREEN_REPLAY_LOADED:
            render_field(&game);
            particles_draw(100);

            {
                const char *banner = "WATCHING REPLAY";
                int bw = MeasureText(banner, 30);
                int bx = FIELD_W / 2 - bw / 2;
                int by = 130;
                DrawRectangle(bx - 20, by - 8, bw + 40, 46,
                              (Color){0, 0, 0, 160});
                DrawText(banner, bx, by, 30, (Color){255, 220, 100, 255});
            }

            DrawText("ESC = exit", FIELD_W / 2 - 60,
                     FIELD_H + 40, 20, LIGHTGRAY);
            break;

        case SCREEN_TIME_ATTACK_OVER:
            render_time_attack_results(ta_score, ta_best);
            break;

        case SCREEN_COACH:
        {
            int winner = (stats.goals_p1 > stats.goals_p2) ? 1 : 2;
            render_coach_screen(&coach, winner);
            break;
        }

        case SCREEN_STATS:
        {
            int winner = (stats.goals_p1 > stats.goals_p2) ? 1 : 2;
            render_stats_screen(&stats, &records, winner);
            break;
        }

        case SCREEN_GAMEOVER:
        {
            int winner = (game.score1 > game.score2) ? 1 : 2;
            render_gameover(&game, winner);
            break;
        }
        }

        EndDrawing();
    }

    audio_close();
    CloseAudioDevice();
    net_close();
    CloseWindow();
    return 0;
}