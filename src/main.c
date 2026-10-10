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
    SCREEN_TIME_ATTACK_OVER
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
    const int menu_n = 6; /* added Time Attack */

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

    /* Time attack state */
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

        /* Global: TAB toggles heatmap, T cycles theme (only when playing) */
        if (screen == SCREEN_PLAYING)
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
                switch (menu_sel)
                {
                case 0:
                    screen = SCREEN_SOLO_DIFF;
                    break;
                case 1:
                    game_init(&game, MODE_LOCAL);
                    powerup_init(&powerups);
                    stats_init(&stats);
                    particles_clear();
                    heatmap_init(&heat);
                    heat.visible = true;
                    screen = SCREEN_PLAYING;
                    break;
                case 2:
                    if (net_host_start())
                        screen = SCREEN_HOST_WAIT;
                    else
                        TraceLog(LOG_ERROR, "Failed to host");
                    break;
                case 3:
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
                    ta_time_left = 60.0f;
                    ta_score = 0;
                    ta_active = true;
                    screen = SCREEN_TIME_ATTACK;
                    break;
                case 5: /* Quit */
                    audio_close();
                    CloseAudioDevice();
                    CloseWindow();
                    return 0;
                }
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

            /* Skill shot spin bias */
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
                float p1x, p1y;
                input_get_local_target(&game, 1, &p1x, &p1y);
                ai_update(&ai, &game, dt);
                float p2x, p2y;
                ai_get_target(&ai, &p2x, &p2y);
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

            stats_update(&stats, &game, dt);

            if (game.ev_hit)
            {
                int hitter = (game.puck.y < FIELD_H / 2.0f) ? 2 : 1;
                stats_on_hit(&stats, hitter);
            }
            if (game.ev_goal)
            {
                stats_on_goal(&stats, game.last_scorer);
                if (is_ta && game.last_scorer == 1)
                    ta_score++;
            }

            /* Heat map */
            heatmap_record(&heat, game.puck.x, game.puck.y);
            heatmap_decay(&heat, dt);

            /* Audio */
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
            }

            /* Particles */
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

            /* Win only in non-time-attack modes */
            if (!is_ta && game_check_win(&game))
            {
                audio_play(SFX_WIN, 1.0f);
                stats.p1_powerups = powerups.p1_pickups;
                stats.p2_powerups = powerups.p2_pickups;
                records_check_and_update(&stats, &records);
                screen = SCREEN_STATS;
            }
            break;
        }

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
                    screen = SCREEN_PLAYING;
                }
                else if (game.mode == MODE_SOLO)
                {
                    game_init(&game, MODE_SOLO);
                    powerup_init(&powerups);
                    stats_init(&stats);
                    particles_clear();
                    heatmap_init(&heat);
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

        /* ================= DRAW ================= */
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
            if (heat.visible)
                heatmap_draw(&heat, 100);
            trajectory_draw(&game, 100, IsKeyDown(KEY_LEFT_CONTROL));
            particles_draw(100);
            powerup_draw(&powerups, 100);
            render_combo_overlay(&stats);
            /* Theme indicator */
            DrawText(TextFormat("Theme: %s (T)", themes_name(themes_current())),
                     10, FIELD_H + 130, 16, GRAY);
            DrawText("TAB: heatmap  CTRL: trajectory  SHIFT: spin",
                     10, FIELD_H + 150, 14, DARKGRAY);
            break;

        case SCREEN_TIME_ATTACK:
            render_field(&game);
            if (heat.visible)
                heatmap_draw(&heat, 100);
            particles_draw(100);
            powerup_draw(&powerups, 100);
            render_combo_overlay(&stats);
            render_time_attack_hud(ta_time_left, ta_score);
            break;

        case SCREEN_TIME_ATTACK_OVER:
            render_time_attack_results(ta_score, ta_best);
            break;

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