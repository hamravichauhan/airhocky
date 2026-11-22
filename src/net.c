#include "net.h"
#include "raylib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

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
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <ifaddrs.h>
typedef int socket_t;
#define INVALID_SOCK (-1)
#define CLOSESOCK close
#endif

#define UDP_BEACON_PORT 50506

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
    return (err == WSAEWOULDBLOCK || err == WSAEINPROGRESS);
}
#else
static inline void init_winsock(void) {}
static inline bool is_would_block(void)
{
    return (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS);
}
#endif

/* ---------------- module state ---------------- */

static socket_t srv_fd = INVALID_SOCK; /* listening socket (host only) */
static socket_t cli_fd = INVALID_SOCK; /* connected socket */
static bool is_host = false;
static bool connected = false;

/* Packet streaming buffer */
static uint8_t s_rx_buf[sizeof(Packet)];
static size_t s_rx_len = 0;

/* Room Code & UDP Beacon */
static char s_room_code[16] = "7429";
static socket_t udp_bcast_fd = INVALID_SOCK;
static socket_t udp_listen_fd = INVALID_SOCK;
static char s_discovered_code[16] = "";
static char s_discovered_ip[64] = "";

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

/* disable Nagle's algorithm for zero latency */
static void set_tcp_nodelay(socket_t fd)
{
    int flag = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (const char *)&flag, sizeof(flag));
}

/* ---------------- IP discovery ---------------- */

const char *net_get_local_ip(void)
{
    static char ip_buf[64] = "127.0.0.1";
#ifdef _WIN32
    init_winsock();
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        struct hostent *he = gethostbyname(hostname);
        if (he != NULL && he->h_addr_list[0] != NULL)
        {
            struct in_addr addr;
            memcpy(&addr, he->h_addr_list[0], sizeof(struct in_addr));
            const char *ip_str = inet_ntoa(addr);
            if (ip_str && strcmp(ip_str, "127.0.0.1") != 0)
                strncpy(ip_buf, ip_str, sizeof(ip_buf) - 1);
        }
    }
#else
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) != -1)
    {
        for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
        {
            if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
                continue;
            if (strcmp(ifa->ifa_name, "lo0") == 0 || strcmp(ifa->ifa_name, "lo") == 0)
                continue;

            struct sockaddr_in *p = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &p->sin_addr, ip_buf, sizeof(ip_buf));
            break;
        }
        freeifaddrs(ifaddr);
    }
#endif
    return ip_buf;
}

/* ---------------- Room Code & Beacon ---------------- */

static double get_real_time(void)
{
    double r_time = GetTime();
    if (r_time > 0.0)
        return r_time;
#ifdef _WIN32
    return (double)GetTickCount() * 0.001;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 1e-6;
#endif
}

const char *net_get_room_code(void)
{
    return s_room_code;
}

void net_beacon_tick(void)
{
    if (udp_bcast_fd == INVALID_SOCK)
        return;

    static double last_time = 0.0;
    double now = get_real_time();
    if (last_time != 0.0 && (now - last_time) < 0.35)
        return;
    last_time = now;

    char msg[128];
    snprintf(msg, sizeof(msg), "AIRHOCKEY:%s:%s", s_room_code, net_get_local_ip());

    struct sockaddr_in baddr;
    memset(&baddr, 0, sizeof(baddr));
    baddr.sin_family = AF_INET;
    baddr.sin_port = htons(UDP_BEACON_PORT);

    /* Broadcast to local LAN / Wi-Fi */
#ifdef _WIN32
    baddr.sin_addr.s_addr = inet_addr("255.255.255.255");
#else
    baddr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
#endif
    sendto(udp_bcast_fd, msg, (int)strlen(msg), 0, (struct sockaddr *)&baddr, sizeof(baddr));

    /* Also send to localhost for same-machine testing */
    baddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sendto(udp_bcast_fd, msg, (int)strlen(msg), 0, (struct sockaddr *)&baddr, sizeof(baddr));
}

void net_discovery_start(void)
{
    init_winsock();
    net_discovery_stop();

    udp_listen_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_listen_fd == INVALID_SOCK)
        return;

    int reuse = 1;
    setsockopt(udp_listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse));
#ifdef SO_REUSEPORT
    setsockopt(udp_listen_fd, SOL_SOCKET, SO_REUSEPORT, (const char *)&reuse, sizeof(reuse));
#endif

    struct sockaddr_in laddr;
    memset(&laddr, 0, sizeof(laddr));
    laddr.sin_family = AF_INET;
    laddr.sin_port = htons(UDP_BEACON_PORT);
    laddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(udp_listen_fd, (struct sockaddr *)&laddr, sizeof(laddr)) < 0)
    {
        CLOSESOCK(udp_listen_fd);
        udp_listen_fd = INVALID_SOCK;
        return;
    }

    set_nonblocking(udp_listen_fd);
    s_discovered_code[0] = '\0';
    s_discovered_ip[0] = '\0';
}

void net_discovery_stop(void)
{
    if (udp_listen_fd != INVALID_SOCK)
    {
        CLOSESOCK(udp_listen_fd);
        udp_listen_fd = INVALID_SOCK;
    }
}

bool net_discovery_poll(char *out_code, char *out_ip)
{
    if (udp_listen_fd == INVALID_SOCK)
        return (s_discovered_ip[0] != '\0');

    char buf[128];
    struct sockaddr_in from;
    socklen_t flen = sizeof(from);
#ifdef _WIN32
    int n = recvfrom(udp_listen_fd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from, &flen);
#else
    ssize_t n = recvfrom(udp_listen_fd, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from, &flen);
#endif
    if (n > 0)
    {
        buf[n] = '\0';
        if (strncmp(buf, "AIRHOCKEY:", 10) == 0)
        {
            char *code = buf + 10;
            char *colon = strchr(code, ':');
            if (colon)
            {
                *colon = '\0';
                char *ip = colon + 1;
                strncpy(s_discovered_code, code, sizeof(s_discovered_code) - 1);
                strncpy(s_discovered_ip, ip, sizeof(s_discovered_ip) - 1);

                if (out_code)
                    strncpy(out_code, s_discovered_code, 15);
                if (out_ip)
                    strncpy(out_ip, s_discovered_ip, 63);
                return true;
            }
        }
    }

    if (s_discovered_ip[0] != '\0')
    {
        if (out_code)
            strncpy(out_code, s_discovered_code, 15);
        if (out_ip)
            strncpy(out_ip, s_discovered_ip, 63);
        return true;
    }

    return false;
}

bool net_resolve_code_or_ip(const char *input, char *out_ip, size_t maxlen)
{
    if (!input || strlen(input) == 0)
    {
        if (s_discovered_ip[0] != '\0')
        {
            strncpy(out_ip, s_discovered_ip, maxlen - 1);
            out_ip[maxlen - 1] = '\0';
            return true;
        }
        return false;
    }

    /* Direct IP entered (e.g. 192.168.1.50) */
    if (strchr(input, '.') != NULL)
    {
        strncpy(out_ip, input, maxlen - 1);
        out_ip[maxlen - 1] = '\0';
        return true;
    }

    /* Matches discovered room code or single active room */
    if (s_discovered_ip[0] != '\0')
    {
        strncpy(out_ip, s_discovered_ip, maxlen - 1);
        out_ip[maxlen - 1] = '\0';
        return true;
    }

    return false;
}

/* send entire buffer */
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

/* receive and reassemble packets without stream corruption */
static bool recv_packet(socket_t fd, Packet *out)
{
    while (s_rx_len < sizeof(Packet))
    {
#ifdef _WIN32
        int n = recv(fd, (char *)(s_rx_buf + s_rx_len), (int)(sizeof(Packet) - s_rx_len), 0);
#else
        ssize_t n = recv(fd, (char *)(s_rx_buf + s_rx_len), sizeof(Packet) - s_rx_len, 0);
#endif
        if (n < 0)
        {
            if (is_would_block())
                return false; /* Wait for remaining bytes on next frame tick */
#ifndef _WIN32
            if (errno == EINTR)
                continue;
#endif
            connected = false;
            return false;
        }
        if (n == 0)
        {
            connected = false;
            s_rx_len = 0;
            return false; /* Remote closed connection */
        }
        s_rx_len += (size_t)n;
    }

    memcpy(out, s_rx_buf, sizeof(Packet));
    s_rx_len = 0;
    return true;
}

/* ---------------- host ---------------- */

bool net_host_start(void)
{
    init_winsock();
    net_close();

    /* Generate a random 4-digit Room Code */
    srand((unsigned int)time(NULL) ^ (unsigned int)GetTime());
    snprintf(s_room_code, sizeof(s_room_code), "%04d", 1000 + (rand() % 9000));

    /* Setup UDP Broadcast beacon */
    udp_bcast_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_bcast_fd != INVALID_SOCK)
    {
        int bcast = 1;
        setsockopt(udp_bcast_fd, SOL_SOCKET, SO_BROADCAST, (const char *)&bcast, sizeof(bcast));
        set_nonblocking(udp_bcast_fd);
    }

    /* Setup TCP Listener */
    srv_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv_fd == INVALID_SOCK)
    {
        TraceLog(LOG_ERROR, "socket() failed");
        return false;
    }

    int opt = 1;
    setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));
    set_tcp_nodelay(srv_fd);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(NET_PORT);

    if (bind(srv_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        TraceLog(LOG_ERROR, "bind() failed on port %d", NET_PORT);
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
    s_rx_len = 0;

    TraceLog(LOG_INFO, "Hosting Room [%s] on IP %s:%d", s_room_code, net_get_local_ip(), NET_PORT);
    return true;
}

/* ---------------- client ---------------- */

bool net_client_connect(const char *ip)
{
    init_winsock();
    net_close();

    if (!ip || strlen(ip) == 0)
        return false;

    cli_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (cli_fd == INVALID_SOCK)
    {
        TraceLog(LOG_ERROR, "socket() failed");
        return false;
    }

    set_tcp_nodelay(cli_fd);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(NET_PORT);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1)
    {
        TraceLog(LOG_ERROR, "Invalid IP format: %s", ip);
        CLOSESOCK(cli_fd);
        cli_fd = INVALID_SOCK;
        return false;
    }

    /* Non-blocking connect with a 2.5-second timeout so window never hangs */
    set_nonblocking(cli_fd);
    int res = connect(cli_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (res < 0)
    {
        if (is_would_block())
        {
            fd_set wfds, efds;
            FD_ZERO(&wfds);
            FD_ZERO(&efds);
            FD_SET(cli_fd, &wfds);
            FD_SET(cli_fd, &efds);

            struct timeval tv;
            tv.tv_sec = 2;
            tv.tv_usec = 500000; /* 2.5s timeout */

            int sel = select((int)(cli_fd + 1), NULL, &wfds, &efds, &tv);
            if (sel <= 0 || FD_ISSET(cli_fd, &efds))
            {
                TraceLog(LOG_ERROR, "Connection timed out to %s:%d", ip, NET_PORT);
                CLOSESOCK(cli_fd);
                cli_fd = INVALID_SOCK;
                return false;
            }

            int so_err = 0;
            socklen_t slen = sizeof(so_err);
            getsockopt(cli_fd, SOL_SOCKET, SO_ERROR, (char *)&so_err, &slen);
            if (so_err != 0)
            {
                TraceLog(LOG_ERROR, "Socket connect error to %s:%d", ip, NET_PORT);
                CLOSESOCK(cli_fd);
                cli_fd = INVALID_SOCK;
                return false;
            }
        }
        else
        {
            TraceLog(LOG_ERROR, "connect() failed immediately");
            CLOSESOCK(cli_fd);
            cli_fd = INVALID_SOCK;
            return false;
        }
    }

    is_host = false;
    connected = true;
    s_rx_len = 0;

    TraceLog(LOG_INFO, "Connected to Host at %s:%d", ip, NET_PORT);
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
    if (udp_bcast_fd != INVALID_SOCK)
    {
        CLOSESOCK(udp_bcast_fd);
        udp_bcast_fd = INVALID_SOCK;
    }
    net_discovery_stop();

    connected = false;
    is_host = false;
    s_rx_len = 0;
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

    /* Accept incoming client connection if waiting */
    if (!connected && srv_fd != INVALID_SOCK)
    {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        socket_t fd = accept(srv_fd, (struct sockaddr *)&caddr, &clen);
        if (fd != INVALID_SOCK)
        {
            set_nonblocking(fd);
            set_tcp_nodelay(fd);
            cli_fd = fd;
            connected = true;
            s_rx_len = 0;

            /* Stop beaconing once client connects */
            if (udp_bcast_fd != INVALID_SOCK)
            {
                CLOSESOCK(udp_bcast_fd);
                udp_bcast_fd = INVALID_SOCK;
            }

            char ipbuf[64];
            inet_ntop(AF_INET, &caddr.sin_addr, ipbuf, sizeof(ipbuf));
            TraceLog(LOG_INFO, "Client successfully connected from %s", ipbuf);

            /* Send MSG_START handshake */
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
    if (!recv_packet(cli_fd, &p))
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

    p.ev_hit = g->ev_hit ? 1 : 0;
    p.ev_wall = g->ev_wall ? 1 : 0;
    p.ev_goal = g->ev_goal ? 1 : 0;
    p.ev_hit_intensity = g->ev_hit_intensity;
    p.ev_wall_intensity = g->ev_wall_intensity;

    return send_all(cli_fd, &p, sizeof(p));
}

bool net_poll_state(GameState *g)
{
    if (!connected || is_host || cli_fd == INVALID_SOCK)
        return false;

    Packet p;
    memset(&p, 0, sizeof(p));
    if (!recv_packet(cli_fd, &p))
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

    g->ev_hit = (p.ev_hit != 0);
    g->ev_wall = (p.ev_wall != 0);
    g->ev_goal = (p.ev_goal != 0);
    g->ev_hit_intensity = p.ev_hit_intensity;
    g->ev_wall_intensity = p.ev_wall_intensity;

    return true;
}
