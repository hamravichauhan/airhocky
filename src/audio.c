#include "audio.h"
#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define SAMPLE_RATE 44100

static Sound s_sounds[SFX_COUNT_TOTAL];
static bool s_loaded = false;

/* ---------- wave builders ---------- */

/* Build a mono wave from a float sample buffer (range -1..1) */
static Wave make_wave(const float *samples, int count)
{
    Wave w = {0};
    w.frameCount = count;
    w.sampleRate = SAMPLE_RATE;
    w.sampleSize = 16;
    w.channels = 1;

    short *data = (short *)MemAlloc(sizeof(short) * count);
    for (int i = 0; i < count; i++)
    {
        float v = samples[i];
        if (v > 1.0f)
            v = 1.0f;
        if (v < -1.0f)
            v = -1.0f;
        data[i] = (short)(v * 32000.0f);
    }
    w.data = data;
    return w;
}

/* Short exponential decay envelope */
static float env_exp(float t, float decay)
{
    return expf(-t * decay);
}

/* ============================================================
   SFX generators
   ============================================================ */

/* Hit — short burst of noise + click, ~40ms */
static Wave gen_hit(void)
{
    int n = SAMPLE_RATE * 40 / 1000;
    float *buf = (float *)MemAlloc(sizeof(float) * n);
    for (int i = 0; i < n; i++)
    {
        float t = (float)i / SAMPLE_RATE;
        float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        float tone = sinf(2 * PI * 900.0f * t); /* quick "pop" */
        buf[i] = (noise * 0.4f + tone * 0.6f) * env_exp(t, 80.0f);
    }
    Wave w = make_wave(buf, n);
    MemFree(buf);
    return w;
}

/* Wall — softer thud, ~50ms, lower pitch */
static Wave gen_wall(void)
{
    int n = SAMPLE_RATE * 50 / 1000;
    float *buf = (float *)MemAlloc(sizeof(float) * n);
    for (int i = 0; i < n; i++)
    {
        float t = (float)i / SAMPLE_RATE;
        float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
        float tone = sinf(2 * PI * 300.0f * t);
        buf[i] = (noise * 0.6f + tone * 0.4f) * env_exp(t, 60.0f);
    }
    Wave w = make_wave(buf, n);
    MemFree(buf);
    return w;
}

/* Goal — rising 3-note fanfare, ~350ms */
static Wave gen_goal(void)
{
    int n = SAMPLE_RATE * 350 / 1000;
    float *buf = (float *)MemAlloc(sizeof(float) * n);
    for (int i = 0; i < n; i++)
    {
        float t = (float)i / SAMPLE_RATE;
        float freq;
        if (t < 0.12f)
            freq = 523.25f; /* C5 */
        else if (t < 0.24f)
            freq = 659.25f; /* E5 */
        else
            freq = 783.99f; /* G5 */
        float tone = sinf(2 * PI * freq * t) * 0.7f + sinf(2 * PI * freq * 2.0f * t) * 0.2f;
        buf[i] = tone * env_exp(t, 5.0f);
    }
    Wave w = make_wave(buf, n);
    MemFree(buf);
    return w;
}

/* Countdown — short beep ~80ms */
static Wave gen_count(void)
{
    int n = SAMPLE_RATE * 80 / 1000;
    float *buf = (float *)MemAlloc(sizeof(float) * n);
    for (int i = 0; i < n; i++)
    {
        float t = (float)i / SAMPLE_RATE;
        float tone = sinf(2 * PI * 440.0f * t);
        buf[i] = tone * env_exp(t, 25.0f) * 0.5f;
    }
    Wave w = make_wave(buf, n);
    MemFree(buf);
    return w;
}

/* GO! — higher beep ~120ms */
static Wave gen_go(void)
{
    int n = SAMPLE_RATE * 120 / 1000;
    float *buf = (float *)MemAlloc(sizeof(float) * n);
    for (int i = 0; i < n; i++)
    {
        float t = (float)i / SAMPLE_RATE;
        float tone = sinf(2 * PI * 880.0f * t);
        buf[i] = tone * env_exp(t, 20.0f) * 0.6f;
    }
    Wave w = make_wave(buf, n);
    MemFree(buf);
    return w;
}

/* Win — triumphant 5-note arpeggio, ~700ms */
static Wave gen_win(void)
{
    int n = SAMPLE_RATE * 700 / 1000;
    float *buf = (float *)MemAlloc(sizeof(float) * n);
    float freqs[5] = {523.25f, 659.25f, 783.99f, 1046.5f, 1318.5f};
    for (int i = 0; i < n; i++)
    {
        float t = (float)i / SAMPLE_RATE;
        int note = (int)(t / 0.14f);
        if (note > 4)
            note = 4;
        float tone = sinf(2 * PI * freqs[note] * t) * 0.6f + sinf(2 * PI * freqs[note] * 2.0f * t) * 0.2f;
        buf[i] = tone * env_exp(t - note * 0.14f, 8.0f);
    }
    Wave w = make_wave(buf, n);
    MemFree(buf);
    return w;
}

/* ---------- public API ---------- */

void audio_init(void)
{
    if (s_loaded)
        return;

    s_sounds[SFX_HIT] = LoadSoundFromWave(gen_hit());
    s_sounds[SFX_WALL] = LoadSoundFromWave(gen_wall());
    s_sounds[SFX_GOAL] = LoadSoundFromWave(gen_goal());
    s_sounds[SFX_COUNT] = LoadSoundFromWave(gen_count());
    s_sounds[SFX_GO] = LoadSoundFromWave(gen_go());
    s_sounds[SFX_WIN] = LoadSoundFromWave(gen_win());

    s_loaded = true;
    TraceLog(LOG_INFO, "Audio: %d sounds generated", SFX_COUNT_TOTAL);
}

void audio_play(SfxId id, float intensity)
{
    if (!s_loaded)
        return;
    if (id < 0 || id >= SFX_COUNT_TOTAL)
        return;

    if (intensity < 0)
        intensity = 0;
    if (intensity > 1)
        intensity = 1;

    Sound *s = &s_sounds[id];

    /* Scale volume & pitch for impact sounds */
    if (id == SFX_HIT || id == SFX_WALL)
    {
        SetSoundVolume(*s, 0.3f + 0.7f * intensity);
        SetSoundPitch(*s, 0.85f + 0.4f * intensity);
    }
    else
    {
        SetSoundVolume(*s, 0.7f);
        SetSoundPitch(*s, 1.0f);
    }

    PlaySound(*s);
}

void audio_close(void)
{
    if (!s_loaded)
        return;
    for (int i = 0; i < SFX_COUNT_TOTAL; i++)
    {
        UnloadSound(s_sounds[i]);
    }
    s_loaded = false;
}