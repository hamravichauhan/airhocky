#ifndef DYNAUDIO_H
#define DYNAUDIO_H

typedef struct
{
    float intensity; /* 0..1 — rises with rally length */
    float target;    /* where we want to be */
} DynamicAudio;

void dynaudio_init(DynamicAudio *d);
void dynaudio_set_intensity(DynamicAudio *d, float intensity);
void dynaudio_update(DynamicAudio *d, float dt);
float dynaudio_get(const DynamicAudio *d);

#endif