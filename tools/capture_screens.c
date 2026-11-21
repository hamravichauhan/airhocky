#include "raylib.h"
#include "game.h"
#include "render.h"
#include "themes.h"
#include "heatmap.h"
#include "particles.h"
#include "powerup.h"
#include "trajectory.h"
#include "stats.h"
#include "coach.h"
#include "settings.h"
#include "settings_ui.h"
#include "tournament.h"
#include "commentary.h"
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
    stats.hype = HYPE_INSANE;
    stats.hype_timer = 2.0f;

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

    CoachReport coach;
    coach_analyze(&coach, &stats, 1);

    Tournament tour;
    tournament_init(&tour, TOUR_BO3);
    tour.wins_p1 = 1;
    tour.wins_p2 = 0;

    Commentary commentary;
    commentary_init(&commentary);
    commentary_trigger(&commentary, CMT_POWER_SHOT);

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
    DrawText("Theme: Neon (T)", 10, FIELD_H + 130, 16, GRAY);
    DrawText("ESC: menu   TAB: heatmap   CTRL: trajectory   SHIFT: spin", 10, FIELD_H + 150, 14, DARKGRAY);
    snap("screenshots/02_gameplay_neon.png");

    // 3. TRAJECTORY & FLICK SHOT
    BeginTextureMode(s_rt);
    ClearBackground((Color){20, 25, 35, 255});
    render_field(&game);
    trajectory_draw(&game, 100, true);
    powerup_draw(&powerups, 100);
    DrawText("TRAJECTORY PREDICTION ACTIVE (CTRL)", FIELD_W / 2 - 160, FIELD_H + 130, 16, YELLOW);
    snap("screenshots/03_trajectory_prediction.png");

    // 4. HEATMAP OVERLAY
    BeginTextureMode(s_rt);
    ClearBackground((Color){20, 25, 35, 255});
    render_field(&game);
    heatmap_draw(&heat, 100);
    DrawText("HEATMAP DENSITY OVERLAY (TAB)", FIELD_W / 2 - 140, FIELD_H + 130, 16, ORANGE);
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

    // 9. LAN MULTIPLAYER JOIN SCREEN
    BeginTextureMode(s_rt);
    ClearBackground((Color){20, 25, 35, 255});
    ui_background();
    ui_title("JOIN GAME", 0.3f);
    ui_subtitle("Enter the host's IP address");
    Rectangle join_panel = {FIELD_W / 2 - 220, 220, 440, 200};
    ui_panel(join_panel);
    DrawText("Host IP:", FIELD_W / 2 - 180, 260, 22, UI_TEXT);
    DrawText("192.168.1.45", FIELD_W / 2 - 180, 300, 26, UI_SUCCESS);
    ui_hint("Type to enter   ENTER = connect   ESC = back");
    ui_back_button_draw();
    snap("screenshots/09_lan_join.png");

    UnloadRenderTexture(s_rt);
    CloseWindow();
    return 0;
}
