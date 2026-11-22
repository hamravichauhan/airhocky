#ifndef NET_H
#define NET_H

#include "protocol.h"
#include "game.h"
#include <stdbool.h>
#include <stddef.h>

/* Lifecycle */
bool net_host_start(void);
bool net_client_connect(const char *ip);
void net_close(void);

/* Local IP discovery */
const char *net_get_local_ip(void);

/* Room Code & Discovery */
const char *net_get_room_code(void);
void net_beacon_tick(void);
void net_discovery_start(void);
void net_discovery_stop(void);
bool net_discovery_poll(char *out_code, char *out_ip);
bool net_resolve_code_or_ip(const char *input, char *out_ip, size_t maxlen);

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