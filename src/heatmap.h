#ifndef HEATMAP_H
#define HEATMAP_H

#include "game.h"
#include "raylib.h"

#define HEAT_COLS 24
#define HEAT_ROWS 32

typedef struct
{
    float grid[HEAT_COLS][HEAT_ROWS]; /* counts per cell */
    float max_val;                    /* for normalization */
    bool visible;                     /* toggle display */
    float alpha;                      /* fade-in/out */
} HeatMap;

void heatmap_init(HeatMap *hm);
void heatmap_record(HeatMap *hm, float x, float y);
void heatmap_decay(HeatMap *hm, float dt); /* slowly fade old data */
void heatmap_draw(const HeatMap *hm, int offset_y);

#endif