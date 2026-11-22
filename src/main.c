#include "raylib.h"
#include "audio.h"
#include "game.h"
#include "render.h"
#include "input.h"
#include "net.h"
#include "solomode.h"
#include "particles.h"
#include "powerup.h"
#include "stats.h"
#include "features.h"
#include "heatmap.h"
#include "themes.h"
#include "trajectory.h"
#include "dynaudio.h"
#include "tournament.h"
#include "replay.h"
#include "settings.h"
#include "settings_ui.h"
#include "ui.h"
#include "ui_transition.h"
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
    SCREEN_REPLAY_LOADED,
    SCREEN_SETTINGS
} Screen;

static AIDifficulty ai_from_settings(const GameSettings *s)
{
    switch (s->ai_difficulty)
    {
    case 0:
        return AI_EASY;
    case 2:
        return AI_HARD;
    default:
        return AI_MEDIUM;
    }
}

int main(void)
{
    InitWindow(FIELD_W, FIELD_H + 120, "Air Hockey");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    srand((unsigned)GetTime() * 1000);

    InitAudioDevice();
    audio_init();
    particles_init();
    themes_init();

    GameSettings settings;
    settings_load(&settings);
    settings_apply(&settings);

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

    DynamicAudio dynaudio;
    dynaudio_init(&dynaudio);

    Tournament tour;
    memset(&tour, 0, sizeof(tour));
    bool in_tournament = false;

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
    bool game_paused = false;
    int pause_confirm = 0; /* 0 = normal pause menu, 1 = confirm restart, 2 = confirm quit */
    int pause_sel = 0;     /* 0 = Resume, 1 = Restart, 2 = Quit */
    int confirm_sel = 1;   /* 0 = Yes, 1 = Cancel */
    bool aim_guide_visible = false;

    char join_ip[64] = "";
    int ip_len = 0;
    char join_error[128] = "";

    UIScreenTransition trans;
    ui_trans_init(&trans);

    float toast_timer = 0;

#define GO(next)                \
    do                          \
    {                           \
        game_paused = false;    \
        pause_confirm = 0;      \
        pause_sel = 0;          \
        confirm_sel = 1;        \
        screen = (next);        \
        ui_trans_begin(&trans); \
    } while (0)

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        if (dt > 0.05f)
            dt = 0.05f;

        ui_trans_update(&trans, dt);

        if (screen == SCREEN_PLAYING || screen == SCREEN_TIME_ATTACK)
        {
            if (IsKeyPressed(KEY_TAB))
                heat.visible = !heat.visible;
            if (IsKeyPressed(KEY_T))
                themes_next();
            if (IsKeyPressed(KEY_C))
                aim_guide_visible = !aim_guide_visible;
        }

        switch (screen)
        {
        case SCREEN_MENU:
        {
            Vector2 m_pos = GetMousePosition();
            bool menu_activate = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);

            for (int i = 0; i < menu_n; i++)
            {
                Rectangle hit = {FIELD_W / 2 - 200, 185 + i * 44 - 6, 400, 38};
                if (CheckCollisionPointRec(m_pos, hit))
                {
                    menu_sel = i;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                        menu_activate = true;
                }
            }

            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                menu_sel = (menu_sel + 1) % menu_n;
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                menu_sel = (menu_sel + menu_n - 1) % menu_n;

            if (menu_activate)
            {
                in_tournament = false;
                switch (menu_sel)
                {
                case 0: /* Solo */
                    GO(SCREEN_SOLO_DIFF);
                    break;
                case 1: /* Local */
                    settings_apply(&settings);
                    game_init(&game, MODE_LOCAL);
                    powerup_init(&powerups);
                    stats_init(&stats);
                    particles_clear();
                    heatmap_init(&heat);
                    heat.visible = true;
                    replay_init(&replay);
                    demo_mode = false;
                    GO(SCREEN_PLAYING);
                    break;
                case 2: /* Host */
                    settings_apply(&settings);
                    if (net_host_start())
                        GO(SCREEN_HOST_WAIT);
                    else
                        TraceLog(LOG_ERROR, "Failed to host");
                    break;
                case 3: /* Join */
                    join_ip[0] = '\0';
                    ip_len = 0;
                    join_error[0] = '\0';
                    net_discovery_start();
                    GO(SCREEN_JOIN_INPUT);
                    break;
                case 4: /* Time Attack */
                    settings_apply(&settings);
                    game_init(&game, MODE_SOLO);
                    ai_init(&ai, ai_from_settings(&settings));
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
                    GO(SCREEN_TIME_ATTACK);
                    break;
                case 5: /* Tournament */
                    settings_apply(&settings);
                    GO(SCREEN_TOURNAMENT_DIFF);
                    break;
                case 6: /* AI vs AI Demo */
                    settings_apply(&settings);
                    game_init(&game, MODE_SOLO);
                    ai_init(&ai, ai_from_settings(&settings));
                    powerup_init(&powerups);
                    stats_init(&stats);
                    particles_clear();
                    heatmap_init(&heat);
                    heat.visible = false;
                    replay_init(&replay);
                    demo_mode = true;
                    GO(SCREEN_PLAYING);
                    break;
                case 7: /* Settings */
                    GO(SCREEN_SETTINGS);
                    break;
                case 8: /* Quit */
                    audio_close();
                    CloseAudioDevice();
                    CloseWindow();
                    return 0;
                }
            }
            break;
        }

        case SCREEN_SETTINGS:
            if (ui_back_button_input())
            {
                settings_save(&settings);
                settings_apply(&settings);
                toast_timer = 1.5f;
                GO(SCREEN_MENU);
                menu_sel = 7;
                break;
            }
            if (settings_ui_update(&settings))
            {
                toast_timer = 1.5f;
                GO(SCREEN_MENU);
                menu_sel = 7;
            }
            break;

        case SCREEN_TOURNAMENT_DIFF:
        {
            if (ui_back_button_input())
            {
                GO(SCREEN_MENU);
                break;
            }
            Vector2 t_mpos = GetMousePosition();
            Rectangle t_box1 = {FIELD_W / 2 - 180, 230, 360, 70};
            Rectangle t_box2 = {FIELD_W / 2 - 180, 350, 360, 70};
            bool click_bo3 = CheckCollisionPointRec(t_mpos, t_box1) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            bool click_bo5 = CheckCollisionPointRec(t_mpos, t_box2) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1) || click_bo3)
            {
                tournament_init(&tour, TOUR_BO3);
                in_tournament = true;
                settings_apply(&settings);
                game_init(&game, MODE_LOCAL);
                powerup_init(&powerups);
                stats_init(&stats);
                particles_clear();
                heatmap_init(&heat);
                heat.visible = true;
                replay_init(&replay);
                demo_mode = false;
                GO(SCREEN_PLAYING);
            }
            if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2) || click_bo5)
            {
                tournament_init(&tour, TOUR_BO5);
                in_tournament = true;
                settings_apply(&settings);
                game_init(&game, MODE_LOCAL);
                powerup_init(&powerups);
                stats_init(&stats);
                particles_clear();
                heatmap_init(&heat);
                heat.visible = true;
                replay_init(&replay);
                demo_mode = false;
                GO(SCREEN_PLAYING);
            }
            break;
        }

        case SCREEN_SOLO_DIFF:
        {
            if (ui_back_button_input())
            {
                GO(SCREEN_MENU);
                break;
            }
            Vector2 s_mpos = GetMousePosition();
            Rectangle s_box1 = {FIELD_W / 2 - 180, 240, 360, 70};
            Rectangle s_box2 = {FIELD_W / 2 - 180, 330, 360, 70};
            Rectangle s_box3 = {FIELD_W / 2 - 180, 420, 360, 70};
            bool click_e = CheckCollisionPointRec(s_mpos, s_box1) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            bool click_m = CheckCollisionPointRec(s_mpos, s_box2) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            bool click_h = CheckCollisionPointRec(s_mpos, s_box3) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1) || click_e)
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
                GO(SCREEN_PLAYING);
            }
            if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2) || click_m)
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
                GO(SCREEN_PLAYING);
            }
            if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3) || click_h)
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
                GO(SCREEN_PLAYING);
            }
            break;
        }

        case SCREEN_HOST_WAIT:
            if (ui_back_button_input())
            {
                net_close();
                GO(SCREEN_MENU);
                break;
            }
            net_beacon_tick();
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
                    GO(SCREEN_PLAYING);
                }
            }
            break;

        case SCREEN_JOIN_INPUT:
            if (ui_back_button_input())
            {
                net_discovery_stop();
                GO(SCREEN_MENU);
                break;
            }
            {
                /* Paste with CTRL+V or CMD+V */
                if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) ||
                     IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) &&
                    IsKeyPressed(KEY_V))
                {
                    const char *cb = GetClipboardText();
                    if (cb && strlen(cb) > 0)
                    {
                        strncpy(join_ip, cb, sizeof(join_ip) - 1);
                        join_ip[sizeof(join_ip) - 1] = '\0';
                        ip_len = (int)strlen(join_ip);
                        join_error[0] = '\0';
                    }
                }

                int key = GetCharPressed();
                while (key > 0)
                {
                    if (key >= 32 && key < 127 && ip_len < 63)
                    {
                        join_ip[ip_len++] = (char)key;
                        join_ip[ip_len] = '\0';
                        join_error[0] = '\0';
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE) && ip_len > 0)
                {
                    join_ip[--ip_len] = '\0';
                    join_error[0] = '\0';
                }

                if (IsKeyPressed(KEY_ENTER))
                {
                    char target_ip[64] = "";
                    if (!net_resolve_code_or_ip(join_ip, target_ip, sizeof(target_ip)))
                    {
                        snprintf(join_error, sizeof(join_error), "Room not found! Enter 4-digit code or Host IP.");
                    }
                    else
                    {
                        settings_apply(&settings);
                        if (net_client_connect(target_ip))
                        {
                            net_discovery_stop();
                            game_init(&game, MODE_CLIENT);
                            powerup_init(&powerups);
                            stats_init(&stats);
                            particles_clear();
                            heatmap_init(&heat);
                            replay_init(&replay);
                            demo_mode = false;
                            GO(SCREEN_PLAYING);
                        }
                        else
                        {
                            snprintf(join_error, sizeof(join_error), "Failed to connect to %s! Check Host.", target_ip);
                        }
                    }
                }
            }
            break;

        case SCREEN_PLAYING:
        case SCREEN_TIME_ATTACK:
        {
            bool is_ta = (screen == SCREEN_TIME_ATTACK);

            Rectangle hud_pause_btn = {FIELD_W - 116, 15, 104, 32};
            Rectangle hud_theme_btn = {12, 15, 104, 32};
            Vector2 mouse_pos = GetMousePosition();
            bool hud_pause_click = CheckCollisionPointRec(mouse_pos, hud_pause_btn) &&
                                   IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            bool hud_theme_click = CheckCollisionPointRec(mouse_pos, hud_theme_btn) &&
                                   IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

            if (hud_theme_click && !game_paused && !demo_mode && !replay_is_active(&replay))
            {
                themes_next();
            }

            if (demo_mode)
            {
                if (IsKeyPressed(KEY_ESCAPE) || hud_pause_click)
                {
                    demo_mode = false;
                    GO(SCREEN_MENU);
                    menu_sel = 0;
                    break;
                }
            }
            else if (!replay_is_active(&replay))
            {
                if (!game_paused)
                {
                    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE) || hud_pause_click)
                    {
                        game_paused = true;
                        pause_confirm = 0;
                        pause_sel = 0;
                        break; /* Immediately stop frame so keypress is not re-consumed below */
                    }
                }
            }

            if (game_paused)
            {
                Rectangle modal_card = {FIELD_W / 2 - 170, 310, 340, 250};

                if (hud_pause_click)
                {
                    game_paused = false;
                    pause_confirm = 0;
                    break;
                }

                if (pause_confirm == 1)
                {
                    /* Confirm Restart Match */
                    Rectangle btn_yes    = {FIELD_W / 2 - 140, modal_card.y + 130, 280, 42};
                    Rectangle btn_cancel = {FIELD_W / 2 - 140, modal_card.y + 185, 280, 42};

                    if (CheckCollisionPointRec(mouse_pos, btn_yes))
                        confirm_sel = 0;
                    if (CheckCollisionPointRec(mouse_pos, btn_cancel))
                        confirm_sel = 1;

                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) ||
                        IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) ||
                        IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_TAB))
                    {
                        confirm_sel = 1 - confirm_sel;
                    }

                    bool click_yes = (CheckCollisionPointRec(mouse_pos, btn_yes) &&
                                     IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ||
                                     IsKeyPressed(KEY_Y) ||
                                     (IsKeyPressed(KEY_ENTER) && confirm_sel == 0);

                    bool click_cancel = (CheckCollisionPointRec(mouse_pos, btn_cancel) &&
                                        IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ||
                                        IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_N) ||
                                        (IsKeyPressed(KEY_ENTER) && confirm_sel == 1);

                    if (click_yes)
                    {
                        game_paused = false;
                        pause_confirm = 0;
                        settings_apply(&settings);
                        if (is_ta)
                        {
                            game_init(&game, MODE_SOLO);
                            ai_init(&ai, ai_from_settings(&settings));
                            powerup_init(&powerups);
                            stats_init(&stats);
                            particles_clear();
                            heatmap_init(&heat);
                            replay_init(&replay);
                            ta_time_left = 60.0f;
                            ta_score = 0;
                            ta_active = true;
                        }
                        else
                        {
                            int cur_mode = game.mode;
                            game_init(&game, cur_mode);
                            if (cur_mode == MODE_SOLO)
                                ai_init(&ai, ai_from_settings(&settings));
                            powerup_init(&powerups);
                            stats_init(&stats);
                            particles_clear();
                            heatmap_init(&heat);
                            replay_init(&replay);
                        }
                        break;
                    }
                    if (click_cancel)
                    {
                        pause_confirm = 0;
                        break;
                    }
                }
                else if (pause_confirm == 2)
                {
                    /* Confirm Quit to Main Menu */
                    Rectangle btn_yes    = {FIELD_W / 2 - 140, modal_card.y + 130, 280, 42};
                    Rectangle btn_cancel = {FIELD_W / 2 - 140, modal_card.y + 185, 280, 42};

                    if (CheckCollisionPointRec(mouse_pos, btn_yes))
                        confirm_sel = 0;
                    if (CheckCollisionPointRec(mouse_pos, btn_cancel))
                        confirm_sel = 1;

                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) ||
                        IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN) ||
                        IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_TAB))
                    {
                        confirm_sel = 1 - confirm_sel;
                    }

                    bool click_yes = (CheckCollisionPointRec(mouse_pos, btn_yes) &&
                                     IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ||
                                     IsKeyPressed(KEY_Y) ||
                                     (IsKeyPressed(KEY_ENTER) && confirm_sel == 0);

                    bool click_cancel = (CheckCollisionPointRec(mouse_pos, btn_cancel) &&
                                        IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ||
                                        IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_N) ||
                                        (IsKeyPressed(KEY_ENTER) && confirm_sel == 1);

                    if (click_yes)
                    {
                        game_paused = false;
                        pause_confirm = 0;
                        net_close();
                        GO(SCREEN_MENU);
                        menu_sel = 0;
                        break;
                    }
                    if (click_cancel)
                    {
                        pause_confirm = 0;
                        break;
                    }
                }
                else
                {
                    /* Standard Pause Menu */
                    Rectangle btn_resume  = {FIELD_W / 2 - 140, modal_card.y + 65,  280, 42};
                    Rectangle btn_restart = {FIELD_W / 2 - 140, modal_card.y + 118, 280, 42};
                    Rectangle btn_quit    = {FIELD_W / 2 - 140, modal_card.y + 171, 280, 42};

                    if (CheckCollisionPointRec(mouse_pos, btn_resume))
                        pause_sel = 0;
                    if (CheckCollisionPointRec(mouse_pos, btn_restart))
                        pause_sel = 1;
                    if (CheckCollisionPointRec(mouse_pos, btn_quit))
                        pause_sel = 2;

                    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                        pause_sel = (pause_sel + 1) % 3;
                    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                        pause_sel = (pause_sel + 2) % 3;

                    bool click_resume = (CheckCollisionPointRec(mouse_pos, btn_resume) &&
                                        IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ||
                                        IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P) ||
                                        IsKeyPressed(KEY_SPACE) ||
                                        (IsKeyPressed(KEY_ENTER) && pause_sel == 0);

                    bool click_restart = (CheckCollisionPointRec(mouse_pos, btn_restart) &&
                                         IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ||
                                         IsKeyPressed(KEY_R) ||
                                         (IsKeyPressed(KEY_ENTER) && pause_sel == 1);

                    bool click_quit = (CheckCollisionPointRec(mouse_pos, btn_quit) &&
                                      IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ||
                                      IsKeyPressed(KEY_M) || IsKeyPressed(KEY_Q) ||
                                      (IsKeyPressed(KEY_ENTER) && pause_sel == 2);

                    if (click_resume)
                    {
                        game_paused = false;
                        break;
                    }
                    if (click_restart)
                    {
                        pause_confirm = 1;
                        confirm_sel = 1;
                        break;
                    }
                    if (click_quit)
                    {
                        pause_confirm = 2;
                        confirm_sel = 1;
                        break;
                    }
                }

                break; /* Freeze physics and timers while game is paused */
            }

            if (replay_is_active(&replay))
            {
                replay_update(&replay, dt);

                if (!replay_is_active(&replay))
                {
                    if (had_replay)
                    {
                        game = game_before_replay;
                        had_replay = false;
                    }
                }
                else
                {
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

                break;
            }

            if (IsKeyPressed(KEY_V))
            {
                game_before_replay = game;
                had_replay = true;
                replay_start(&replay);
                if (!replay_is_active(&replay))
                    had_replay = false;
                break;
            }

            if (IsKeyPressed(KEY_F5))
            {
                if (replay_save(&replay.buf, "replay.replay"))
                    toast_timer = 1.5f;
            }

            if (is_ta && ta_active)
            {
                ta_time_left -= dt;
                if (ta_time_left <= 0)
                {
                    ta_time_left = 0;
                    ta_active = false;
                    if (ta_score > ta_best)
                        ta_best = ta_score;
                    GO(SCREEN_TIME_ATTACK_OVER);
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
                        p1y = game.puck.y - 30.0f;
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
                    GO(SCREEN_MENU);
                    net_close();
                }
            }
            else if (game.mode == MODE_CLIENT)
            {
                float p2x, p2y;
                input_get_local_target(&game, 2, &p2x, &p2y);
                net_send_input(p2x, p2y);
                while (net_poll_state(&game))
                {
                }
                if (!net_is_connected())
                {
                    GO(SCREEN_MENU);
                    net_close();
                }
            }

            replay_record(&replay, &game);

            stats_update(&stats, &game, dt);

            if (game.ev_hit)
            {
                int hitter = (game.puck.y < FIELD_H / 2.0f) ? 2 : 1;
                stats_on_hit(&stats, hitter);
                ai_record_player_hit(&ai, game.puck.x);
            }
            if (game.ev_goal)
            {
                stats_on_goal(&stats, game.last_scorer);
                ai_record_player_goal(&ai, game.puck.x);
                if (is_ta && game.last_scorer == 1)
                    ta_score++;
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
            }

            float puck_speed = sqrtf(game.puck.vx * game.puck.vx +
                                     game.puck.vy * game.puck.vy);
            if (puck_speed > 80.0f && game.reset_timer <= 0)
                particles_trail_add((Vector2){game.puck.x, game.puck.y});

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
                        GO(SCREEN_STATS);
                    }
                    else
                    {
                        settings_apply(&settings);
                        game_init(&game, MODE_LOCAL);
                        powerup_init(&powerups);
                        stats_init(&stats);
                        particles_clear();
                        heatmap_init(&heat);
                        heat.visible = true;
                        replay_init(&replay);
                        GO(SCREEN_PLAYING);
                    }
                }
                else
                {
                    GO(SCREEN_STATS);
                }
            }
            break;
        }

        case SCREEN_REPLAY_LOADED:
        {
            if (ui_back_button_input())
            {
                GO(SCREEN_MENU);
                menu_sel = 8;
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

        case SCREEN_TIME_ATTACK_OVER:
            if (ui_back_button_input())
            {
                GO(SCREEN_MENU);
                menu_sel = 0;
                break;
            }
            if (IsKeyPressed(KEY_R))
            {
                settings_apply(&settings);
                game_init(&game, MODE_SOLO);
                ai_init(&ai, ai_from_settings(&settings));
                powerup_init(&powerups);
                stats_init(&stats);
                particles_clear();
                heatmap_init(&heat);
                heat.visible = true;
                replay_init(&replay);
                ta_time_left = 60.0f;
                ta_score = 0;
                ta_active = true;
                GO(SCREEN_TIME_ATTACK);
            }
            break;

        case SCREEN_STATS:
            if (ui_back_button_input())
            {
                net_close();
                GO(SCREEN_MENU);
                menu_sel = 0;
                break;
            }
            if (IsKeyPressed(KEY_R))
            {
                net_close();
                if (game.mode == MODE_LOCAL)
                {
                    settings_apply(&settings);
                    game_init(&game, MODE_LOCAL);
                    powerup_init(&powerups);
                    stats_init(&stats);
                    particles_clear();
                    heatmap_init(&heat);
                    replay_init(&replay);
                    GO(SCREEN_PLAYING);
                }
                else if (game.mode == MODE_SOLO)
                {
                    settings_apply(&settings);
                    game_init(&game, MODE_SOLO);
                    powerup_init(&powerups);
                    stats_init(&stats);
                    particles_clear();
                    heatmap_init(&heat);
                    replay_init(&replay);
                    GO(SCREEN_PLAYING);
                }
                else
                {
                    GO(SCREEN_MENU);
                }
            }
            break;

        case SCREEN_GAMEOVER:
            if (ui_back_button_input())
            {
                net_close();
                GO(SCREEN_MENU);
                menu_sel = 0;
                break;
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

        case SCREEN_SETTINGS:
            settings_ui_draw(&settings);
            ui_back_button_draw();
            break;

        case SCREEN_SOLO_DIFF:
            render_solo_difficulty(menu_sel);
            ui_back_button_draw();
            break;

        case SCREEN_TOURNAMENT_DIFF:
            ui_background();
            ui_title("TOURNAMENT FORMAT", 0.3f);
            ui_subtitle("How many wins to take the series?");
            {
                Rectangle panel = {FIELD_W / 2 - 220, 180, 440, 380};
                ui_panel(panel);
                DrawText("1  -  Best of 3", FIELD_W / 2 - 90, 240, 30, UI_SUCCESS);
                DrawText("First to 2 match wins", FIELD_W / 2 - 90, 278, 16, UI_TEXT_DIM);
                DrawText("2  -  Best of 5", FIELD_W / 2 - 90, 360, 30, UI_ACCENT_HOT);
                DrawText("First to 3 match wins", FIELD_W / 2 - 90, 398, 16, UI_TEXT_DIM);
            }
            ui_hint("1/2 to choose   ESC = back");
            ui_back_button_draw();
            break;

        case SCREEN_HOST_WAIT:
            render_host_wait_screen(net_get_room_code(), net_get_local_ip());
            ui_back_button_draw();
            break;

        case SCREEN_JOIN_INPUT:
            ui_background();
            ui_title("JOIN LAN GAME", 0.3f);
            ui_subtitle("Enter the 4-digit Room Code to connect");
            {
                Rectangle panel = {FIELD_W / 2 - 240, 180, 480, 310};
                ui_panel(panel);

                char d_code[16] = "", d_ip[64] = "";
                bool has_room = net_discovery_poll(d_code, d_ip);

                DrawText("ROOM CODE / HOST IP:", FIELD_W / 2 - 200, 210, 18, UI_TEXT_DIM);

                Rectangle box = {FIELD_W / 2 - 200, 240, 400, 56};
                DrawRectangleRec(box, (Color){15, 25, 40, 255});
                DrawRectangleLinesEx(box, 2.0f, UI_ACCENT);

                if (ip_len == 0)
                {
                    if (has_room)
                    {
                        DrawText(TextFormat("Found: %s (Press ENTER)", d_code), (int)box.x + 15, (int)box.y + 14, 24, UI_SUCCESS);
                    }
                    else
                    {
                        DrawText("Enter 4-digit code (e.g. 7429)", (int)box.x + 15, (int)box.y + 16, 22, DARKGRAY);
                    }
                }
                else
                {
                    DrawText(join_ip, (int)box.x + 15, (int)box.y + 12, 30, UI_ACCENT_HOT);
                }

                /* Blinking cursor */
                if ((int)(GetTime() * 2.5f) % 2 == 0)
                {
                    int cw = MeasureText(join_ip, 30);
                    DrawRectangle((int)box.x + 18 + cw, (int)box.y + 12, 3, 32, UI_ACCENT_HOT);
                }

                if (join_error[0] != '\0')
                {
                    DrawText(join_error, FIELD_W / 2 - 200, 310, 15, (Color){255, 90, 90, 255});
                }
                else if (has_room)
                {
                    DrawText(TextFormat("Room [%s] detected on local Wi-Fi!", d_code),
                             FIELD_W / 2 - 200, 310, 15, UI_SUCCESS);
                }
                else
                {
                    DrawText("Searching for rooms on local Wi-Fi...", FIELD_W / 2 - 200, 310, 14, UI_TEXT_DIM);
                }

                DrawText("Type Code -> Press ENTER to join",
                         FIELD_W / 2 - 200, 350, 16, RAYWHITE);
                DrawText("ENTER = Connect   CTRL+V = Paste   BACKSPACE = Delete",
                         FIELD_W / 2 - 200, 400, 13, UI_TEXT_DARK);
            }
            ui_hint("Press ESC to return to menu");
            ui_back_button_draw();
            break;

        case SCREEN_PLAYING:
        case SCREEN_TIME_ATTACK:
        {
            bool is_ta = (screen == SCREEN_TIME_ATTACK);
            render_field(&game);
            if (heat.visible && !replay_is_active(&replay))
                heatmap_draw(&heat, 100);
            if (!is_ta && !replay_is_active(&replay))
            {
                bool show_aim = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL) ||
                                IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER) ||
                                IsKeyDown(KEY_C) || aim_guide_visible;
                trajectory_draw(&game, 100, show_aim);
            }
            particles_draw(100);
            if (!replay_is_active(&replay))
                powerup_draw(&powerups, 100);
            if (!is_ta && in_tournament)
                render_tournament_hud(&tour);
            if (!replay_is_active(&replay))
                render_combo_overlay(&stats);
            if (is_ta && !replay_is_active(&replay))
                render_time_attack_hud(ta_time_left, ta_score);
            render_replay_banner(replay_is_active(&replay), 0);

            /* --- Top Scoreboard Controls (Inside y = 0 .. 100, never covering field or goal) --- */
            Rectangle hud_pause_btn = {FIELD_W - 116, 15, 104, 32};
            Rectangle hud_theme_btn = {12, 15, 104, 32};
            Vector2 m_pos = GetMousePosition();
            bool hud_pause_hover = CheckCollisionPointRec(m_pos, hud_pause_btn);
            bool hud_theme_hover = CheckCollisionPointRec(m_pos, hud_theme_btn);

            if (demo_mode)
            {
                ui_button(hud_pause_btn, "EXIT (ESC)", hud_pause_hover);
                const char *dtxt = "DEMO MODE (AI vs AI) - ESC to Exit";
                int dtw = MeasureText(dtxt, 11);
                DrawText(dtxt, FIELD_W / 2 - dtw / 2, 78, 11, YELLOW);
            }
            else if (replay_is_active(&replay))
            {
                ui_button(hud_pause_btn, "RESUME (P)", hud_pause_hover);
                const char *rtxt = "SLOW-MO REPLAY (0.35x)";
                int rtw = MeasureText(rtxt, 11);
                DrawText(rtxt, FIELD_W / 2 - rtw / 2, 78, 11, UI_ACCENT_HOT);
            }
            else
            {
                /* Top-Left Theme toggle button */
                ui_button(hud_theme_btn, themes_name(themes_current()), hud_theme_hover);

                /* Top-Right Pause button */
                const char *btn_lbl = game_paused ? "RESUME (P)" : "PAUSE (P)";
                ui_button(hud_pause_btn, btn_lbl, hud_pause_hover);

                /* Top-Center subtle shortcut hint at y = 78 (between P1 and P2 scores) */
                const char *hints = "ESC / P: Pause   TAB: Heatmap   CTRL / C: Aim";
                int htw = MeasureText(hints, 10);
                DrawText(hints, FIELD_W / 2 - htw / 2, 78, 10, UI_TEXT_DIM);
            }

            /* --- Bottom Margin Status Strip (y = 907 .. 920, strictly BELOW Player 1's goal) --- */
            DrawLine(0, 905, FIELD_W, 905, (Color){30, 45, 65, 180});
            const char *ctrl_hint = "WASD / Arrows: Move Paddle   |   SHIFT: Spin   |   V: Slow-Mo Replay   |   T: Theme";
            int ctw = MeasureText(ctrl_hint, 10);
            DrawText(ctrl_hint, FIELD_W / 2 - ctw / 2, 908, 10, UI_TEXT_DARK);

            /* --- Pause Modal Overlay --- */
            if (game_paused)
            {
                DrawRectangle(0, 0, FIELD_W, FIELD_H + 120, (Color){5, 10, 20, 215});

                Rectangle modal_card = {FIELD_W / 2 - 170, 310, 340, 250};
                ui_panel(modal_card);

                if (pause_confirm == 1)
                {
                    const char *ptitle = "RESTART MATCH?";
                    int ptw = MeasureText(ptitle, 24);
                    DrawText(ptitle, FIELD_W / 2 - ptw / 2, modal_card.y + 18, 24, UI_ACCENT_HOT);

                    const char *sub1 = "Are you sure you want to restart?";
                    int stw1 = MeasureText(sub1, 13);
                    DrawText(sub1, FIELD_W / 2 - stw1 / 2, modal_card.y + 55, 13, UI_TEXT);

                    const char *sub2 = "Current score and rally will be reset.";
                    int stw2 = MeasureText(sub2, 11);
                    DrawText(sub2, FIELD_W / 2 - stw2 / 2, modal_card.y + 78, 11, UI_TEXT_DIM);

                    Rectangle btn_yes    = {FIELD_W / 2 - 140, modal_card.y + 130, 280, 42};
                    Rectangle btn_cancel = {FIELD_W / 2 - 140, modal_card.y + 185, 280, 42};

                    ui_button(btn_yes,    "YES, RESTART (ENTER / Y)", (confirm_sel == 0) || CheckCollisionPointRec(m_pos, btn_yes));
                    ui_button(btn_cancel, "CANCEL (ESC / N)",         (confirm_sel == 1) || CheckCollisionPointRec(m_pos, btn_cancel));
                }
                else if (pause_confirm == 2)
                {
                    const char *ptitle = "QUIT TO MENU?";
                    int ptw = MeasureText(ptitle, 24);
                    DrawText(ptitle, FIELD_W / 2 - ptw / 2, modal_card.y + 18, 24, UI_ACCENT_HOT);

                    const char *sub1 = "Are you sure you want to quit?";
                    int stw1 = MeasureText(sub1, 13);
                    DrawText(sub1, FIELD_W / 2 - stw1 / 2, modal_card.y + 55, 13, UI_TEXT);

                    const char *sub2 = "Unsaved match progress will be lost.";
                    int stw2 = MeasureText(sub2, 11);
                    DrawText(sub2, FIELD_W / 2 - stw2 / 2, modal_card.y + 78, 11, UI_TEXT_DIM);

                    Rectangle btn_yes    = {FIELD_W / 2 - 140, modal_card.y + 130, 280, 42};
                    Rectangle btn_cancel = {FIELD_W / 2 - 140, modal_card.y + 185, 280, 42};

                    ui_button(btn_yes,    "YES, QUIT TO MENU (ENTER / Y)", (confirm_sel == 0) || CheckCollisionPointRec(m_pos, btn_yes));
                    ui_button(btn_cancel, "CANCEL (ESC / N)",              (confirm_sel == 1) || CheckCollisionPointRec(m_pos, btn_cancel));
                }
                else
                {
                    const char *ptitle = "MATCH PAUSED";
                    int ptw = MeasureText(ptitle, 24);
                    DrawText(ptitle, FIELD_W / 2 - ptw / 2, modal_card.y + 18, 24, UI_ACCENT_HOT);

                    Rectangle btn_resume  = {FIELD_W / 2 - 140, modal_card.y + 65,  280, 42};
                    Rectangle btn_restart = {FIELD_W / 2 - 140, modal_card.y + 118, 280, 42};
                    Rectangle btn_quit    = {FIELD_W / 2 - 140, modal_card.y + 171, 280, 42};

                    ui_button(btn_resume,  "Resume Match (ESC / P)", (pause_sel == 0) || CheckCollisionPointRec(m_pos, btn_resume));
                    ui_button(btn_restart, "Restart Match (R)",      (pause_sel == 1) || CheckCollisionPointRec(m_pos, btn_restart));
                    ui_button(btn_quit,    "Quit to Main Menu (M)",  (pause_sel == 2) || CheckCollisionPointRec(m_pos, btn_quit));
                }
            }
            break;
        }

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
            ui_hint("ESC = exit");
            ui_back_button_draw();
            break;

        case SCREEN_TIME_ATTACK_OVER:
            render_time_attack_results(ta_score, ta_best);
            ui_back_button_draw();
            break;

        case SCREEN_STATS:
        {
            int winner = (stats.goals_p1 > stats.goals_p2) ? 1 : 2;
            render_stats_screen(&stats, &records, winner);
            ui_back_button_draw();
            break;
        }

        case SCREEN_GAMEOVER:
        {
            int winner = (game.score1 > game.score2) ? 1 : 2;
            render_gameover(&game, winner);
            ui_back_button_draw();
            break;
        }
        }

        if (toast_timer > 0)
            ui_toast("Settings saved!", &toast_timer);

        ui_trans_draw(&trans);

        EndDrawing();
    }

#undef GO

    audio_close();
    CloseAudioDevice();
    net_close();
    CloseWindow();
    return 0;
}