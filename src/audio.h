#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

typedef enum
{
    SFX_HIT,   /* paddle contact (volume scaled) */
    SFX_WALL,  /* wall bounce */
    SFX_GOAL,  /* scoring fanfare */
    SFX_COUNT, /* countdown tick (3,2,1) */
    SFX_GO,    /* "GO!" beep */
    SFX_WIN,   /* victory fanfare */
    SFX_COUNT_TOTAL
} SfxId;

/* Init — call once at program start (after InitAudioDevice) */
void audio_init(void);

/* Play a sound. For SFX_HIT/SFX_WALL, 'intensity' (0..1) scales volume & pitch. */
void audio_play(SfxId id, float intensity);

/* Clean up */
void audio_close(void);

#endif