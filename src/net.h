#ifndef NET_H
#define NET_H

#include "protocol.h"
#include "game.h"
#include <stdbool.h>

/* Lifecycle */
bool net_host_start(void);
bool net_client_connect(const char *ip);
void net_close(void);

/* State */
bool net_is_connected(void);
bool net_is_host(void);

/* Client -> host: send my paddle target */
bool net_send_input(float tx, float ty);

/* Host side: poll for client input (non-blocking). Returns true if got one. */
bool net_poll_input(float *tx, float *ty);

/* Host -> client: send full state snapshot */
bool net_send_state(const GameState *g);

/* Client side: receive state snapshot (non-blocking) */
bool net_poll_state(GameState *g);

#endif