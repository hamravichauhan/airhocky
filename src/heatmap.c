#include "heatmap.h"
#include "features.h"
#include <math.h>
#include <string.h>

#if FEATURE_HEATMAP

void heatmap_init(HeatMap *hm)
{
    memset(hm, 0, sizeof(*hm));
    hm->max_val = 1.0f;
    hm->visible = false;
    hm->alpha = 0.0f;
}

void heatmap_record(HeatMap *hm, float x, float y)
{
    if (x < 0 || y < 0 || x >= FIELD_W || y >= FIELD_H)
        return;

    int cx = (int)(x / FIELD_W * HEAT_COLS);
    int cy = (int)(y / FIELD_H * HEAT_ROWS);
    if (cx < 0)
        cx = 0;
    if (cx >= HEAT_COLS)
        cx = HEAT_COLS - 1;
    if (cy < 0)
        cy = 0;
    if (cy >= HEAT_ROWS)
        cy = HEAT_ROWS - 1;

    hm->grid[cx][cy] += 1.0f;
    if (hm->grid[cx][cy] > hm->max_val)
        hm->max_val = hm->grid[cx][cy];
}

void heatmap_decay(HeatMap *hm, float dt)
{
    /* very slow decay so old paths fade over minutes */
    float k = 1.0f - dt * 0.02f;
    if (k < 0.99f)
        k = 0.99f;

    float m = 1.0f;
    for (int x = 0; x < HEAT_COLS; x++)
    {
        for (int y = 0; y < HEAT_ROWS; y++)
        {
            hm->grid[x][y] *= k;
            if (hm->grid[x][y] > m)
                m = hm->grid[x][y];
        }
    }
    hm->max_val = m;

    /* fade the alpha toward target */
    float target = hm->visible ? 1.0f : 0.0f;
    hm->alpha += (target - hm->alpha) * (dt * 6.0f);
}

void heatmap_draw(const HeatMap *hm, int offset_y)
{
    if (hm->alpha < 0.02f)
        return;
    if (hm->max_val < 0.5f)
        return;

    float cell_w = (float)FIELD_W / HEAT_COLS;
    float cell_h = (float)FIELD_H / HEAT_ROWS;

    for (int x = 0; x < HEAT_COLS; x++)
    {
        for (int y = 0; y < HEAT_ROWS; y++)
        {
            float v = hm->grid[x][y] / hm->max_val;
            if (v < 0.05f)
                continue;

            /* Blue (low) → Red (high) */
            unsigned char r = (unsigned char)(255 * v);
            unsigned char g = (unsigned char)(100 * (1.0f - v) * v * 4);
            unsigned char b = (unsigned char)(255 * (1.0f - v));
            unsigned char a = (unsigned char)(120 * v * hm->alpha);

            DrawRectangle(
                (int)(x * cell_w),
                (int)(y * cell_h) + offset_y,
                (int)cell_w + 1,
                (int)cell_h + 1,
                (Color){r, g, b, a});
        }
    }
}

#else /* disabled */

void heatmap_init(HeatMap *hm) { (void)hm; }
void heatmap_record(HeatMap *hm, float x, float y)
{
    (void)hm;
    (void)x;
    (void)y;
}
void heatmap_decay(HeatMap *hm, float dt)
{
    (void)hm;
    (void)dt;
}
void heatmap_draw(const HeatMap *hm, int offset_y)
{
    (void)hm;
    (void)offset_y;
}

#endif