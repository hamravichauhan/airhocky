#include "raylib.h"
#include "game.h"
#include "render.h"
#include "ai.h"
#include "particles.h"
#include "powerup.h"
#include "stats.h"
#include "heatmap.h"
#include "themes.h"
#include "trajectory.h"
#include "commentary.h"
#include "replay.h"
#include "settings.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void record_sequence(const char *filename, int total_frames,
                            bool show_trajectory, bool show_heatmap,
                            bool cycle_themes, bool trigger_replay)
{
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -f rawvideo -pix_fmt rgba -s %dx%d -r 60 -i - -c:v libx264 -preset fast -crf 18 -pix_fmt yuv420p %s 2>/dev/null",
             FIELD_W, FIELD_H + 120, filename);

    FILE *ffmpeg = popen(cmd, "w");
    if (!ffmpeg)
    {
        printf("Failed to open ffmpeg for %s\n", filename);
        return;
    }

    printf("Recording %s (%d frames)...\n", filename, total_frames);

    GameState game;
    game_init(&game, MODE_SOLO);
    game.reset_timer = 0.0f; // immediately active

    // Give puck initial motion
    game.puck.vx = 450.0f;
    game.puck.vy = -380.0f;
    game.puck_spin = 10.0f;

    AIController ai;
    ai_init(&ai, AI_HARD);

    PowerUpState powerups;
    powerup_init(&powerups);
    powerups.drop1.active = true;
    powerups.drop1.type = PU_GROW;
    powerups.drop1.x = 200;
    powerups.drop1.y = 520;

    powerups.drop2.active = true;
    powerups.drop2.type = PU_FREEZE;
    powerups.drop2.x = 400;
    powerups.drop2.y = 280;

    MatchStats stats;
    stats_init(&stats);

    HeatMap heat;
    heatmap_init(&heat);
    heat.visible = show_heatmap;

    Commentary commentary;
    commentary_init(&commentary);

    Replay replay;
    replay_init(&replay);

    float dt = 1.0f / 60.0f;

    for (int frame = 0; frame < total_frames; frame++)
    {
        if (cycle_themes && frame % 120 == 0)
        {
            themes_next();
        }

        // P2 AI (Top)
        ai_update(&ai, &game, dt);
        float p2x, p2y;
        ai_get_target(&ai, &p2x, &p2y);

        // P1 AI (Bottom - Smooth Tracker)
        float p1x = game.puck.x + ((frame % 30 < 15) ? 20.0f : -20.0f);
        float p1y = FIELD_H - 120.0f;
        if (game.puck.y > FIELD_H / 2.0f + 40.0f && game.puck.y < FIELD_H - 60.0f)
        {
            p1y = game.puck.y + 40.0f;
        }

        // If goal scored, ensure game resets after short delay
        if (game.reset_timer > 0.0f)
        {
            game.reset_timer -= dt;
            if (game.reset_timer <= 0.0f)
            {
                game.puck.vx = (frame % 2 == 0) ? 500.0f : -500.0f;
                game.puck.vy = (frame % 4 < 2) ? 420.0f : -420.0f;
            }
        }
        else
        {
            game_update(&game, dt, p1x, p1y, p2x, p2y);
        }

        particles_update(dt);
        stats_update(&stats, &game, dt);
        commentary_update(&commentary, dt);
        powerup_update(&powerups, &game, dt, 1);
        heatmap_record(&heat, game.puck.x, game.puck.y);
        replay_record(&replay, &game);

        if (game.ev_hit)
        {
            particles_emit_spark((Vector2){game.puck.x, game.puck.y},
                                 (Vector2){game.ev_hit_dir_x, game.ev_hit_dir_y},
                                 game.ev_hit_intensity);
            if (game.ev_hit_intensity > 0.7f)
                commentary_trigger(&commentary, CMT_POWER_SHOT);
            else if (stats.current_rally == 10)
                commentary_trigger(&commentary, CMT_RALLY_10);
        }
        if (game.ev_goal)
        {
            particles_emit_goal((Vector2){game.puck.x, game.puck.y}, RED);
            commentary_trigger(&commentary, CMT_INSANE);
        }

        BeginDrawing();
        ClearBackground((Color){20, 25, 35, 255});

        render_field(&game);

        if (show_heatmap)
        {
            heatmap_draw(&heat, 100);
        }
        if (show_trajectory)
        {
            trajectory_draw(&game, 100, true);
        }

        particles_draw(100);
        powerup_draw(&powerups, 100);
        render_combo_overlay(&stats);
        commentary_draw(&commentary, 100);

        if (trigger_replay && frame > 180 && frame < 320)
        {
            render_replay_banner(true, 320 - frame);
        }

        // Draw HUD Footer
        DrawText(TextFormat("Theme: %s", themes_name(themes_current())),
                 10, FIELD_H + 130, 16, GRAY);
        DrawText("AI Simulation • 60 FPS Continuous Collision Engine",
                 10, FIELD_H + 150, 14, DARKGRAY);

        EndDrawing();

        Image img = LoadImageFromScreen();
        fwrite(img.data, 1, FIELD_W * (FIELD_H + 120) * 4, ffmpeg);
        UnloadImage(img);
    }

    pclose(ffmpeg);
    printf("Saved: %s\n", filename);
}

int main(void)
{
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(FIELD_W, FIELD_H + 120, "Clip Recorder");
    SetTargetFPS(60);

    themes_init();
    particles_init();

    // 1. High-Speed Rally & Goal Celebration (8 sec = 480 frames)
    themes_set_by_index(THEME_NEON);
    record_sequence("video_assets/clips/01_fast_rally_goal.mp4", 480, false, false, false, false);

    // 2. Trajectory Prediction & Flick Shots (6.5 sec = 390 frames)
    themes_set_by_index(THEME_FIRE);
    record_sequence("video_assets/clips/02_trajectory_flick.mp4", 390, true, false, false, false);

    // 3. Live Heatmap Analytics (6 sec = 360 frames)
    themes_set_by_index(THEME_DARK);
    record_sequence("video_assets/clips/03_heatmap_active.mp4", 360, false, true, false, false);

    // 4. Color Theme Showcase Cycling (8 sec = 480 frames)
    record_sequence("video_assets/clips/04_theme_showcase.mp4", 480, false, false, true, false);

    // 5. Instant Slow-Mo Replay (6 sec = 360 frames)
    themes_set_by_index(THEME_RETRO);
    record_sequence("video_assets/clips/05_slowmo_replay.mp4", 360, false, false, false, true);

    CloseWindow();
    printf("All 5 gameplay video clips recorded successfully!\n");
    return 0;
}
