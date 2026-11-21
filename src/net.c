#include "net.h"
#include "raylib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET socket_t;
#define INVALID_SOCK INVALID_SOCKET
#define CLOSESOCK closesocket
#else
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
typedef int socket_t;
#define INVALID_SOCK (-1)
#define CLOSESOCK close
#endif

/* ---------------- winsock & error helpers ---------------- */

#ifdef _WIN32
static bool winsock_ready = false;
static void init_winsock(void)
{
    if (!winsock_ready)
    {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) == 0)
            winsock_ready = true;
    }
}
static inline bool is_would_block(void)
{
    int err = WSAGetLastError();
    return (err == WSAEWOULDBLOCK);
}
#else
static inline void init_winsock(void) {}
static inline bool is_would_block(void)
{
    return (errno == EAGAIN || errno == EWOULDBLOCK);
}
#endif

/* ---------------- module state ---------------- */

static socket_t srv_fd = INVALID_SOCK; /* listening socket (host only) */
static socket_t cli_fd = INVALID_SOCK; /* connected socket */
static bool is_host = false;
static bool connected = false;

/* set socket non-blocking */
static void set_nonblocking(socket_t fd)
{
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(fd, FIONBIO, &mode);
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0)
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#endif
}

/* send entire buffer (handles partial sends) */
static bool send_all(socket_t fd, const void *buf, size_t len)
{
    const char *p = (const char *)buf;
    size_t sent = 0;
    while (sent < len)
    {
#ifdef _WIN32
        int n = send(fd, p + sent, (int)(len - sent), 0);
#else
        ssize_t n = send(fd, p + sent, len - sent, 0);
#endif
        if (n < 0)
        {
            if (is_would_block())
                continue;
            return false;
        }
        if (n == 0)
            return false;
        sent += (size_t)n;
    }
    return true;
}

/* receive exactly len bytes into buf (non-blocking) */
static bool recv_all(socket_t fd, void *buf, size_t len)
{
    char *p = (char *)buf;
    size_t got = 0;
    while (got < len)
    {
#ifdef _WIN32
        int n = recv(fd, p + got, (int)(len - got), 0);
#else
        ssize_t n = recv(fd, p + got, len - got, 0);
#endif
        if (n < 0)
        {
            if (is_would_block())
                return false; /* not ready */
#ifndef _WIN32
            if (errno == EINTR)
                continue;
#endif
            return false;
        }
        if (n == 0)
        {
            connected = false;
            return false;
        } /* peer closed */
        got += (size_t)n;
    }
    return true;
}

/* ---------------- host ---------------- */

bool net_host_start(void)
{
    init_winsock();
    net_close();

    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv_fd == INVALID_SOCK)
    {
        TraceLog(LOG_ERROR, "socket() failed");
        return false;
    }

    int opt = 1;
    setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(NET_PORT);

    if (bind(srv_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        TraceLog(LOG_ERROR, "bind() failed");
        CLOSESOCK(srv_fd);
        srv_fd = INVALID_SOCK;
        return false;
    }

    if (listen(srv_fd, 1) < 0)
    {
        TraceLog(LOG_ERROR, "listen() failed");
        CLOSESOCK(srv_fd);
        srv_fd = INVALID_SOCK;
        return false;
    }

    set_nonblocking(srv_fd);

    is_host = true;
    connected = false;

    TraceLog(LOG_INFO, "Hosting on port %d — waiting for client", NET_PORT);
    return true;
}

/* ---------------- client ---------------- */

bool net_client_connect(const char *ip)
{
    init_winsock();
    net_close();

    cli_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_fd == INVALID_SOCK)
    {
        TraceLog(LOG_ERROR, "socket() failed");
        return false;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NET_PORT);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1)
    {
        TraceLog(LOG_ERROR, "Invalid IP: %s", ip);
        CLOSESOCK(cli_fd);
        cli_fd = INVALID_SOCK;
        return false;
    }

    if (connect(cli_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        TraceLog(LOG_ERROR, "connect() failed");
        CLOSESOCK(cli_fd);
        cli_fd = INVALID_SOCK;
        return false;
    }

    set_nonblocking(cli_fd);

    is_host = false;
    connected = true;

    TraceLog(LOG_INFO, "Connected to %s:%d", ip, NET_PORT);
    return true;
}

/* ---------------- common ---------------- */

void net_close(void)
{
    if (cli_fd != INVALID_SOCK)
    {
        CLOSESOCK(cli_fd);
        cli_fd = INVALID_SOCK;
    }
    if (srv_fd != INVALID_SOCK)
    {
        CLOSESOCK(srv_fd);
        srv_fd = INVALID_SOCK;
    }
    connected = false;
    is_host = false;
}

bool net_is_connected(void) { return connected; }
bool net_is_host(void) { return is_host; }

/* ---------------- client -> host ---------------- */

bool net_send_input(float tx, float ty)
{
    if (!connected || is_host || cli_fd == INVALID_SOCK)
        return false;
    Packet p;
    memset(&p, 0, sizeof(p));
    p.type = MSG_INPUT;
    p.client_tx = tx;
    p.client_ty = ty;
    return send_all(cli_fd, &p, sizeof(p));
}

/* ---------------- host side: poll for client input + accept new client ---------------- */

bool net_poll_input(float *tx, float *ty)
{
    if (!is_host)
        return false;

    /* Try to accept a new client if we don't have one */
    if (!connected && srv_fd != INVALID_SOCK)
    {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        socket_t fd = accept(srv_fd, (struct sockaddr *)&caddr, &clen);
        if (fd != INVALID_SOCK)
        {
            set_nonblocking(fd);
            cli_fd = fd;
            connected = true;
            char ipbuf[64];
            inet_ntop(AF_INET, &caddr.sin_addr, ipbuf, sizeof(ipbuf));
            TraceLog(LOG_INFO, "Client connected from %s", ipbuf);

            /* Send MSG_START so client knows we're rolling */
            Packet start;
            memset(&start, 0, sizeof(start));
            start.type = MSG_START;
            send_all(cli_fd, &start, sizeof(start));
        }
    }

    if (!connected || cli_fd == INVALID_SOCK)
        return false;

    Packet p;
    memset(&p, 0, sizeof(p));
    if (!recv_all(cli_fd, &p, sizeof(p)))
        return false;
    if (p.type != MSG_INPUT)
        return false;

    *tx = p.client_tx;
    *ty = p.client_ty;
    return true;
}

/* ---------------- host -> client ---------------- */

bool net_send_state(const GameState *g)
{
    if (!connected || !is_host || cli_fd == INVALID_SOCK)
        return false;

    Packet p;
    memset(&p, 0, sizeof(p));
    p.type = MSG_STATE;
    p.p1_x = g->p1.x;
    p.p1_y = g->p1.y;
    p.p2_x = g->p2.x;
    p.p2_y = g->p2.y;
    p.puck_x = g->puck.x;
    p.puck_y = g->puck.y;
    p.puck_vx = g->puck.vx;
    p.puck_vy = g->puck.vy;
    p.puck_spin = g->puck_spin;
    p.score1 = (uint8_t)g->score1;
    p.score2 = (uint8_t)g->score2;
    p.reset_timer = g->reset_timer;
    p.goal_flash = g->goal_flash;
    p.last_scorer = (uint8_t)g->last_scorer;
    p.shake = g->shake;

    return send_all(cli_fd, &p, sizeof(p));
}

bool net_poll_state(GameState *g)
{
    if (!connected || is_host || cli_fd == INVALID_SOCK)
        return false;

    Packet p;
    memset(&p, 0, sizeof(p));
    if (!recv_all(cli_fd, &p, sizeof(p)))
        return false;
    if (p.type != MSG_STATE)
        return false;

    g->p1.x = p.p1_x;
    g->p1.y = p.p1_y;
    g->p2.x = p.p2_x;
    g->p2.y = p.p2_y;
    g->puck.x = p.puck_x;
    g->puck.y = p.puck_y;
    g->puck.vx = p.puck_vx;
    g->puck.vy = p.puck_vy;
    g->puck_spin = p.puck_spin;
    g->score1 = p.score1;
    g->score2 = p.score2;
    g->reset_timer = p.reset_timer;
    g->goal_flash = p.goal_flash;
    g->last_scorer = p.last_scorer;
    g->shake = p.shake;
    return true;
}