#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define NET_PORT 50505
#define NET_MAX_PLAYERS 2

enum MsgType
{
    MSG_HELLO = 1,
    MSG_START = 2,
    MSG_INPUT = 3,
    MSG_STATE = 4,
    MSG_BYE = 5
};

typedef struct
{
    uint8_t type;

    /* client -> host: paddle target */
    float client_tx;
    float client_ty;

    /* host -> client: full snapshot */
    float p1_x, p1_y;
    float p2_x, p2_y;
    float puck_x, puck_y;
    float puck_vx, puck_vy;
    float puck_spin;
    uint8_t score1, score2;
    float reset_timer;
    float goal_flash;
    uint8_t last_scorer;
    float shake;
} Packet;

#endif