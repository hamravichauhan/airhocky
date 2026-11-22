#include "raylib.h"
#include "game.h"
#include "render.h"
#include "themes.h"
#include "heatmap.h"
#include "particles.h"
#include "powerup.h"
#include "trajectory.h"
#include "stats.h"
#include "settings.h"
#include "settings_ui.h"
#include "tournament.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>

static RenderTexture2D s_rt;

static void snap(const char *path)
{
    EndTextureMode();
    Image img = LoadImageFromTexture(s_rt.texture);
    ImageFlipVertical(&img);
    ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8);
    ExportImage(img, path);
    UnloadImage(img);
    printf("Saved screenshot: %s\n", path);
}

static void draw_hud(const char *theme_name, int pause_mode)
{
    /* Scoreboard controls (inside y = 0 .. 100) */
    Rectangle hud_pause_btn = {FIELD_W - 116, 15, 104, 32};
    Rectangle hud_theme_btn = {12, 15, 104, 32};

    ui_button(hud_theme_btn, theme_name, false);
    ui_button(hud_pause_btn, pause_mode > 0 ? "RESUME (P)" : "PAUSE (P)", false);

    const char *hints = "ESC: Pause   TAB: Heatmap   CTRL: Aim";
    int htw = MeasureText(hints, 10);
    DrawText(hints, FIELD_W / 2 - htw / 2, 78, 10, UI_TEXT_DIM);

    /* Bottom strip below Player 1's goal (y = 905 .. 920) */
    DrawLine(0, 905, FIELD_W, 905, (Color){30, 45, 65, 180});
    const char *ctrl_hint = "WASD / Arrows: Move Paddle   |   SHIFT: Spin   |   V: Slow-Mo Replay";
    int ctw = MeasureText(ctrl_hint, 10);
    DrawText(ctrl_hint, FIELD_W / 2 - ctw / 2, 908, 10, UI_TEXT_DARK);

    if (pause_mode > 0)
    {
        DrawRectangle(0, 0, FIELD_W, FIELD_H + 120, (Color){5, 10, 20, 215});
        Rectangle modal_card = {FIELD_W / 2 - 170, 310, 340, 250};
        ui_panel(modal_card);

        if (pause_mode == 2)
        {
            /* Confirm Quit Modal */
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

            ui_button(btn_yes,    "YES, QUIT TO MENU (ENTER / Y)", false);
            ui_button(btn_cancel, "CANCEL (ESC / N)",              false);
        }
        else
        {
            /* Standard Pause Modal */
            const char *ptitle = "MATCH PAUSED";
            int ptw = MeasureText(ptitle, 24);
            DrawText(ptitle, FIELD_W / 2 - ptw / 2, modal_card.y + 18, 24, UI_ACCENT_HOT);

            Rectangle btn_resume  = {FIELD_W / 2 - 140, modal_card.y + 65,  280, 42};
            Rectangle btn_restart = {FIELD_W / 2 - 140, modal_card.y + 118, 280, 42};
            Rectangle btn_quit    = {FIELD_W / 2 - 140, modal_card.y + 171, 280, 42};

            ui_button(btn_resume,  "Resume Match (ESC / P)", false);
            ui_button(btn_restart, "Restart Match (R)",      false);
            ui_button(btn_quit,    "Quit to Main Menu (M)",  false);
        }
    }
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(FIELD_W, FIELD_H + 120, "Air Hockey Screenshots");
    SetTargetFPS(60);

    s_rt = LoadRenderTexture(FIELD_W, FIELD_H + 120);

    themes_init();
    particles_init();

    GameSettings settings;
    settings_load(&settings);
    settings_apply(&settings);

    GameState game;
    game_init(&game, MODE_LOCAL);
    game.reset_timer = 0.0f; // in-action
    game.score1 = 3;
    game.score2 = 2;
    game.p1.x = 240;
    game.p1.y = 650;
    game.p2.x = 360;
    game.p2.y = 220;
    game.puck.x = 280;
    game.puck.y = 480;
    game.puck.vx = 420;
    game.puck.vy = -380;
    game.puck_spin = 15.0f;

    PowerUpState powerups;
    powerup_init(&powerups);
    powerups.drop1.active = true;
    powerups.drop1.type = PU_GROW;
    powerups.drop1.x = 180;
    powerups.drop1.y = 520;
    powerups.drop1.pulse = 1.0f;

    powerups.drop2.active = true;
    powerups.drop2.type = PU_FREEZE;
    powerups.drop2.x = 420;
    powerups.drop2.y = 280;
    powerups.drop2.pulse = 1.0f;

    MatchStats stats;
    stats_init(&stats);
    stats.current_rally = 14;
    stats.best_rally = 18;
    stats.p1_hits = 24;
    stats.p2_hits = 21;
    stats.goals_p1 = 5;
    stats.goals_p2 = 3;
    stats.goals_conceded_p1 = 3;
    stats.goals_conceded_p2 = 5;
    stats.top_puck_speed = 1120.0f;
    stats.match_time = 142.5f;
    stats.hype = HYPE_NONE;
    stats.hype_timer = 0.0f;

    Records records;
    records.best_rally_ever = 26;
    records.fastest_puck_ever = 1380.0f;
    records.most_goals_p1_ever = 7;
    records.most_goals_p2_ever = 7;
    records.matches_played = 42;

    HeatMap heat;
    heatmap_init(&heat);
    heat.visible = true;
    for (int i = 0; i < 200; i++) {
        heatmap_record(&heat, 200 + (i * 13) % 200, 300 + (i * 17) % 350);
    }

    Tournament tour;
    tournament_init(&tour, TOUR_BO3);
    tour.wins_p1 = 1;
    tour.wins_p2 = 0;

    // 1. MAIN MENU
    BeginTextureMode(s_rt);
    ClearBackground((Color){20, 25, 35, 255});
    render_menu(0);
    snap("screenshots/01_main_menu.png");

    // 2. GAMEPLAY - NEON THEME
    themes_set_by_index(THEME_NEON);
    BeginTextureMode(s_rt);
    ClearBackground((Color){20, 25, 35, 255});
    render_field(&game);
    powerup_draw(&powerups, 100);
    particles_emit_spark((Vector2){280, 480}, (Vector2){1, -1}, 0.8f);
    particles_draw(100);
    render_combo_overlay(&stats);
    draw_hud("Neon", 0);
    snap("screenshots/02_gameplay_neon.png");

    // 3. TRAJECTORY & FLICK SHOT
    BeginTextureMode(s_rt);
    ClearBackground((Color){20, 25, 35, 255});
    render_field(&game);
    trajectory_draw(&game, 100, true);
    powerup_draw(&powerups, 100);
    draw_hud("Neon", 0);
    snap("screenshots/03_trajectory_prediction.png");

    // 4. HEATMAP OVERLAY
    BeginTextureMode(s_rt);
    ClearBackground((Color){20, 25, 35, 255});
    render_field(&game);
    heatmap_draw(&heat, 100);
    draw_hud("Neon", 0);
    snap("screenshots/04_heatmap_overlay.png");

    // 5. MATCH STATISTICS SCREEN
    BeginTextureMode(s_rt);
    ClearBackground((Color){20, 25, 35, 255});
    render_stats_screen(&stats, &records, 1);
    snap("screenshots/05_match_stats.png");

    // 6. TIME ATTACK RESULTS
    BeginTextureMode(s_rt);
    ClearBackground((Color){20, 25, 35, 255});
    render_time_attack_results(18, 18);
    snap("screenshots/06_time_attack.png");

    // 7. SETTINGS MENU
    BeginTextureMode(s_rt);
    ClearBackground((Color){20, 25, 35, 255});
    settings_ui_draw(&settings);
    ui_back_button_draw();
    snap("screenshots/07_settings_menu.png");

    // 8. TOURNAMENT FORMAT SELECTION
    BeginTextureMode(s_rt);
    ClearBackground((Color){20, 25, 35, 255});
    ui_background();
    ui_title("TOURNAMENT FORMAT", 0.3f);
    ui_subtitle("How many wins to take the series?");
    Rectangle panel = {FIELD_W / 2 - 220, 180, 440, 380};
    ui_panel(panel);
    DrawText("1  -  Best of 3", FIELD_W / 2 - 90, 240, 30, UI_SUCCESS);
    DrawText("First to 2 match wins", FIELD_W / 2 - 90, 278, 16, UI_TEXT_DIM);
    DrawText("2  -  Best of 5", FIELD_W / 2 - 90, 360, 30, UI_ACCENT_HOT);
    DrawText("First to 3 match wins", FIELD_W / 2 - 90, 398, 16, UI_TEXT_DIM);
    ui_hint("1/2 to choose   ESC = back");
    ui_back_button_draw();
    snap("screenshots/08_tournament_mode.png");

    // 9. LAN MULTIPLAYER JOIN SCREEN (ROOM CODE)
    BeginTextureMode(s_rt);
    ClearBackground((Color){20, 25, 35, 255});
    ui_background();
    ui_title("JOIN LAN GAME", 0.3f);
    ui_subtitle("Enter the 4-digit Room Code to connect");
    Rectangle panel9 = {FIELD_W / 2 - 240, 180, 480, 310};
    ui_panel(panel9);
    DrawText("ROOM CODE / HOST IP:", FIELD_W / 2 - 200, 210, 18, UI_TEXT_DIM);
    Rectangle box9 = {FIELD_W / 2 - 200, 240, 400, 56};
    DrawRectangleRec(box9, (Color){15, 25, 40, 255});
    DrawRectangleLinesEx(box9, 2.0f, UI_ACCENT);
    DrawText("7429", (int)box9.x + 15, (int)box9.y + 12, 30, UI_ACCENT_HOT);
    DrawText("Room [7429] detected on local Wi-Fi!", FIELD_W / 2 - 200, 310, 15, UI_SUCCESS);
    DrawText("Type Code -> Press ENTER to join", FIELD_W / 2 - 200, 350, 16, RAYWHITE);
    DrawText("ENTER = Connect   CTRL+V = Paste   BACKSPACE = Delete", FIELD_W / 2 - 200, 400, 13, UI_TEXT_DARK);
    ui_hint("Press ESC to return to menu");
    ui_back_button_draw();
    snap("screenshots/09_lan_join.png");

    // 10. IN-GAME PAUSE & NAVIGATION MODAL
    BeginTextureMode(s_rt);
    ClearBackground((Color){20, 25, 35, 255});
    render_field(&game);
    powerup_draw(&powerups, 100);
    render_combo_overlay(&stats);
    draw_hud("Neon", 1);
    snap("screenshots/10_pause_modal.png");

    // 11. IN-GAME QUIT/RESTART CONFIRMATION MODAL
    BeginTextureMode(s_rt);
    ClearBackground((Color){20, 25, 35, 255});
    render_field(&game);
    powerup_draw(&powerups, 100);
    render_combo_overlay(&stats);
    draw_hud("Neon", 2);
    snap("screenshots/11_pause_confirm.png");

    UnloadRenderTexture(s_rt);
    CloseWindow();
    return 0;
}
