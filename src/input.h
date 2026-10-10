#ifndef INPUT_H
#define INPUT_H

#include "game.h"

/* Get the *target* X,Y that this machine wants its paddle to move toward.
   out_x, out_y are filled. paddle_id: 1 for P1, 2 for P2. */
void input_get_local_target(const GameState *g, int paddle_id,
                            float *out_x, float *out_y);

#endif