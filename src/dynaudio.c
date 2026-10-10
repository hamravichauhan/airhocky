#include "dynaudio.h"
#include "features.h"
#include <string.h>

#if FEATURE_DYNAMIC_AUDIO

void dynaudio_init(DynamicAudio *d)
{
    memset(d, 0, sizeof(*d));
    d->intensity = 0.0f;
    d->target = 0.0f;
}

void dynaudio_set_intensity(DynamicAudio *d, float intensity)
{
    if (intensity < 0)
        intensity = 0;
    if (intensity > 1)
        intensity = 1;
    d->target = intensity;
}

void dynaudio_update(DynamicAudio *d, float dt)
{
    /* smooth ramp */
    float k = dt * 2.5f;
    if (k > 1.0f)
        k = 1.0f;
    d->intensity += (d->target - d->intensity) * k;
}

float dynaudio_get(const DynamicAudio *d)
{
    return d->intensity;
}

#else
void dynaudio_init(DynamicAudio *d) { memset(d, 0, sizeof(*d)); }
void dynaudio_set_intensity(DynamicAudio *d, float i)
{
    (void)d;
    (void)i;
}
void dynaudio_update(DynamicAudio *d, float dt)
{
    (void)d;
    (void)dt;
}
float dynaudio_get(const DynamicAudio *d)
{
    (void)d;
    return 0.0f;
}
#endif