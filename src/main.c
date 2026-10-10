#include "raylib.h"
#include "game.h"
#include "render.h"
#include "input.h"
#include "net.h"
#include <stdio.h>
#include <string.h>

typedef enum
{
    SCREEN_MENU,
    SCREEN_HOST_WAIT,
    SCREEN_JOIN_INPUT,
    SCREEN_PLAYING,
    SCREEN_GAMEOVER
} Screen;

int main(void)
{
    InitWindow(FIELD_W, FIELD_H + 120, "Air Hockey");
    SetTargetFPS(60);

    Screen screen = SCREEN_MENU;
    int menu_sel = 0;
    const int menu_n = 4;

    GameState game;
    memset(&game, 0, sizeof(game));

    char join_ip[64] = "127.0.0.1";
    int ip_len = 9;

    while (!WindowShouldClose())
    {
        /* ================= UPDATE ================= */
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
                    game_init(&game, MODE_LOCAL);
                    screen = SCREEN_PLAYING;
                    break;
                case 1:
                    if (net_host_start())
                        screen = SCREEN_HOST_WAIT;
                    else
                        TraceLog(LOG_ERROR, "Failed to host");
                    break;
                case 2:
                    screen = SCREEN_JOIN_INPUT;
                    break;
                case 3:
                    CloseWindow();
                    return 0;
                }
            }
            break;

        case SCREEN_HOST_WAIT:
            /* Try accepting a client; if accepted, start game */
            {
                float tx, ty;
                if (net_poll_input(&tx, &ty) || net_is_connected())
                {
                    /* once connected, begin */
                    if (net_is_connected())
                    {
                        game_init(&game, MODE_HOST);
                        screen = SCREEN_PLAYING;
                    }
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
        {
            float dt = GetFrameTime();
            if (dt > 0.05f)
                dt = 0.05f;

            if (game.mode == MODE_LOCAL)
            {
                float p1x, p1y, p2x, p2y;
                input_get_local_target(&game, 1, &p1x, &p1y);
                input_get_local_target(&game, 2, &p2x, &p2y);
                game_update(&game, dt, p1x, p1y, p2x, p2y);
            }
            else if (game.mode == MODE_HOST)
            {
                /* Simulate locally, take P2 target from network */
                float p1x, p1y;
                input_get_local_target(&game, 1, &p1x, &p1y);

                float p2x = game.p2.x, p2y = game.p2.y;
                float rx, ry;
                while (net_poll_input(&rx, &ry))
                {
                    p2x = rx;
                    p2y = ry; /* last one wins */
                }

                game_update(&game, dt, p1x, p1y, p2x, p2y);

                /* Broadcast snapshot to client */
                net_send_state(&game);

                if (!net_is_connected())
                {
                    /* client disconnected */
                    screen = SCREEN_MENU;
                    net_close();
                }
            }
            else if (game.mode == MODE_CLIENT)
            {
                /* Read input + send to host */
                float p1x, p1y;
                input_get_local_target(&game, 1, &p1x, &p1y);
                net_send_input(p1x, p1y);

                /* Read state — drain queue, use most recent */
                while (net_poll_state(&game))
                { /* keep last */
                }

                if (!net_is_connected())
                {
                    screen = SCREEN_MENU;
                    net_close();
                }
            }

            if (game_check_win(&game))
                screen = SCREEN_GAMEOVER;
            break;
        }

        case SCREEN_GAMEOVER:
            if (IsKeyPressed(KEY_ENTER))
            {
                net_close();
                screen = SCREEN_MENU;
                menu_sel = 0;
            }
            break;
        }

        /* ================= DRAW ================= */
        BeginDrawing();
        ClearBackground((Color){20, 25, 35, 255});

        switch (screen)
        {
        case SCREEN_MENU:
            render_menu(menu_sel);
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
            break;

        case SCREEN_GAMEOVER:
        {
            int winner = (game.score1 > game.score2) ? 1 : 2;
            render_gameover(&game, winner);
            break;
        }
        }

        EndDrawing();
    }

    net_close();
    CloseWindow();
    return 0;
}