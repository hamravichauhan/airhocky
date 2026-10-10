#ifndef REPLAY_H
#define REPLAY_H

#include "game.h"
#include <stdbool.h>

#define REPLAY_FRAMES 300 /* 5 seconds at 60 FPS */
#define REPLAY_RATE 0.35f /* slow-mo speed factor */

typedef struct
{
    float p1x, p1y;
    float p2x, p2y;
    float puck_x, puck_y;
    float puck_vx, puck_vy;
    float spin;
} ReplayFrame;

typedef struct
{
    ReplayFrame frames[REPLAY_FRAMES];
    int head;  /* next write index */
    int count; /* valid frames (max REPLAY_FRAMES) */
} ReplayBuffer;

typedef struct
{
    bool active;        /* in replay playback? */
    int play_idx;       /* current frame index */
    int play_start;     /* first frame to replay */
    float frame_timer;  /* accumulates real time between frames */
    float frame_period; /* time between frames at REPLAY_RATE */
    ReplayBuffer buf;
} Replay;

void replay_init(Replay *r);
void replay_record(Replay *r, const GameState *g);
void replay_start(Replay *r);
void replay_update(Replay *r, float dt);
bool replay_is_active(const Replay *r);

/* Get the frame the current playback is on — writes into *out */
void replay_get_current(const Replay *r, ReplayFrame *out);

/* File I/O */
bool replay_save(const ReplayBuffer *buf, const char *filename);
bool replay_load(ReplayBuffer *buf, const char *filename);

#endif