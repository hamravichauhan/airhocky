#include "replay.h"
#include "features.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

#if FEATURE_REPLAY

void replay_init(Replay *r)
{
    memset(r, 0, sizeof(*r));
    r->frame_period = (1.0f / 60.0f) / REPLAY_RATE;
    r->play_idx = -1;
}

void replay_record(Replay *r, const GameState *g)
{
    ReplayFrame *f = &r->buf.frames[r->buf.head];
    f->p1x = g->p1.x;
    f->p1y = g->p1.y;
    f->p2x = g->p2.x;
    f->p2y = g->p2.y;
    f->puck_x = g->puck.x;
    f->puck_y = g->puck.y;
    f->puck_vx = g->puck.vx;
    f->puck_vy = g->puck.vy;
    f->spin = g->puck_spin;

    r->buf.head = (r->buf.head + 1) % REPLAY_FRAMES;
    if (r->buf.count < REPLAY_FRAMES)
        r->buf.count++;
}

void replay_start(Replay *r)
{
    if (r->buf.count < 30)
        return; /* need at least 0.5s of data */

    r->active = true;
    r->play_idx = 0;

    /* Start replay from the oldest frame in the buffer */
    if (r->buf.count < REPLAY_FRAMES)
    {
        r->play_start = 0;
    }
    else
    {
        r->play_start = r->buf.head; /* oldest frame is at head */
    }
    r->frame_timer = 0;
}

void replay_update(Replay *r, float dt)
{
    if (!r->active)
        return;

    r->frame_timer += dt;
    while (r->frame_timer >= r->frame_period)
    {
        r->frame_timer -= r->frame_period;
        r->play_idx++;
        if (r->play_idx >= r->buf.count)
        {
            r->active = false;
            r->play_idx = -1;
            return;
        }
    }
}

bool replay_is_active(const Replay *r)
{
    return r->active;
}

void replay_get_current(const Replay *r, ReplayFrame *out)
{
    if (!r->active)
        return;

    int idx = (r->play_start + r->play_idx) % REPLAY_FRAMES;
    *out = r->buf.frames[idx];
}

/* ---------------- File I/O ---------------- */

bool replay_save(const ReplayBuffer *buf, const char *filename)
{
    FILE *f = fopen(filename, "wb");
    if (!f)
        return false;

    /* Write a small header: magic + count */
    const char magic[4] = {'A', 'H', 'R', 'P'};
    unsigned int count = (unsigned int)buf->count;
    unsigned int head = (unsigned int)buf->head;

    fwrite(magic, 1, 4, f);
    fwrite(&count, sizeof count, 1, f);
    fwrite(&head, sizeof head, 1, f);
    fwrite(buf->frames, sizeof(ReplayFrame), REPLAY_FRAMES, f);

    fclose(f);
    return true;
}

bool replay_load(ReplayBuffer *buf, const char *filename)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
        return false;

    char magic[4];
    unsigned int count = 0, head = 0;

    if (fread(magic, 1, 4, f) != 4)
    {
        fclose(f);
        return false;
    }
    if (memcmp(magic, "AHRP", 4) != 0)
    {
        fclose(f);
        return false;
    }
    if (fread(&count, sizeof count, 1, f) != 1)
    {
        fclose(f);
        return false;
    }
    if (fread(&head, sizeof head, 1, f) != 1)
    {
        fclose(f);
        return false;
    }
    if (fread(buf->frames, sizeof(ReplayFrame), REPLAY_FRAMES, f) != REPLAY_FRAMES)
    {
        fclose(f);
        return false;
    }

    buf->count = (int)count;
    buf->head = (int)head;
    if (buf->count > REPLAY_FRAMES)
        buf->count = REPLAY_FRAMES;

    fclose(f);
    return true;
}

#else
/* stubs so build still works when feature disabled */
void replay_init(Replay *r) { memset(r, 0, sizeof(*r)); }
void replay_record(Replay *r, const GameState *g)
{
    (void)r;
    (void)g;
}
void replay_start(Replay *r) { (void)r; }
void replay_update(Replay *r, float dt)
{
    (void)r;
    (void)dt;
}
bool replay_is_active(const Replay *r)
{
    (void)r;
    return false;
}
void replay_get_current(const Replay *r, ReplayFrame *out)
{
    (void)r;
    (void)out;
}
bool replay_save(const ReplayBuffer *b, const char *fn)
{
    (void)b;
    (void)fn;
    return false;
}
bool replay_load(ReplayBuffer *b, const char *fn)
{
    (void)b;
    (void)fn;
    return false;
}
#endif