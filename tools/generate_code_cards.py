import os
from pygments import highlight
from pygments.lexers import CLexer
from pygments.formatters import ImageFormatter

OUT_DIR = "/Users/ravichauhan/Desktop/COLLEGE/C/PROJ/video_assets/code_snippets"
os.makedirs(OUT_DIR, exist_ok=True)

snippets = [
    ("01_networking_nonblocking.png", """/* src/net.c - Non-Blocking POSIX TCP Sockets & Stream Handling */

static void set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0)
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

/* send_all: handles partial socket transfers without dropping bytes */
static bool send_all(int fd, const void *buf, size_t len)
{
    const char *p = (const char *)buf;
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, p + sent, len - sent, 0);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue; /* OS send buffer full: retry next turn */
            return false;
        }
        if (n == 0) return false;
        sent += (size_t)n;
    }
    return true;
}"""),

    ("02_physics_substepping.png", """/* src/game.c - Continuous Collision Detection (CCD) Sub-stepping */

static void integrate_puck(GameState *g, float dt, float r1, float r2)
{
    float speed = sqrtf(len2(g->puck.vx, g->puck.vy));
    
    /* Dynamically slice frame: displacement per step <= 0.5 * radius */
    int steps = 1 + (int)(speed * dt / (PUCK_R * 0.5f));
    if (steps > MAX_SUBSTEPS) steps = MAX_SUBSTEPS; /* Clamp: 1..6 */
    float sdt = dt / steps;

    for (int i = 0; i < steps; i++) {
        /* Advance position by micro-interval */
        g->puck.x += g->puck.vx * sdt;
        g->puck.y += g->puck.vy * sdt;

        /* Collision checks execute at high temporal resolution */
        collide_paddle(g, &g->puck, &g->puck_spin, &g->p1, r1);
        collide_paddle(g, &g->puck, &g->puck_spin, &g->p2, r2);
        
        /* Cushion and goal slot resolution ... */
    }
}"""),

    ("03_magnus_spin_flick.png", """/* src/game.c - Magnus Effect Flight Curvature & Flick Momentum Boost */

/* 1. Flick Shot Boost: Fast paddle swings transfer momentum */
float paddle_speed = sqrtf(len2(pad->vx, pad->vy));
if (paddle_speed > 50.0f) {
    float boost = 0.35f * paddle_speed;
    puck->vx += nx * boost;
    puck->vy += ny * boost;
}

/* 2. Magnus Effect: Rotational spin curves the trajectory in flight */
float sp = g->puck_spin;
if (fabsf(sp) > 0.01f) {
    float curve = sp * 0.0006f;
    float px = g->puck.vy, py = -g->puck.vx; /* Perpendicular normal vector */
    float pl = sqrtf(len2(px, py)) + 0.0001f;
    g->puck.vx += (px / pl) * curve;
    g->puck.vy += (py / pl) * curve;
    g->puck_spin *= 0.985f; /* Exponential angular damping */
}"""),

    ("04_ai_fsm_adaptive.png", """/* src/ai.c - Autonomous FSM & Adaptive Player Habit Tracking */

void ai_update(AIController *ai, const GameState *g, float dt)
{
    switch (ai->state) {
    case AI_STATE_DEFEND:
        /* Adaptive positioning: bias resting arc toward player scoring habits */
        float bias = (ai->p1_goals_left > ai->p1_goals_right) ? -35.0f : 35.0f;
        ai->target_x = (FIELD_W / 2.0f) + bias;
        if (g->puck.y < FIELD_H / 2.0f) ai->state = AI_STATE_INTERCEPT;
        break;

    case AI_STATE_ATTACK:
        /* Windup step before strike */
        ai->strike_x = ai->target_x;
        ai->strike_y = ai->target_y - 20.0f; /* 20px recoil step */
        ai->state = AI_STATE_STRIKE;
        break;

    case AI_STATE_STRIKE:
        /* Explosive forward thrust aimed at goal opening */
        ai->target_y += 500.0f * dt;
        break;
    }
}"""),

    ("05_procedural_audio_dsp.png", """/* src/audio.c - 100% Mathematical Procedural Waveform Synthesis (DSP) */

#define SAMPLE_RATE 44100

static Wave gen_hit(void)
{
    int n = SAMPLE_RATE * 40 / 1000; /* 40 millisecond impulse burst */
    float *buf = (float *)MemAlloc(sizeof(float) * n);
    
    for (int i = 0; i < n; i++) {
        float t = (float)i / SAMPLE_RATE;
        /* Fundamental pitch sweep (240Hz -> 80Hz) + exponential decay */
        float freq = 240.0f - 160.0f * (t / 0.04f);
        float env = expf(-t * 90.0f);
        buf[i] = sinf(2.0f * PI * freq * t) * env;
    }
    
    /* Convert floating-point buffer directly into 16-bit PCM raylib Wave */
    Wave w = make_wave(buf, n);
    MemFree(buf);
    return w;
}""")
]

formatter = ImageFormatter(
    font_name="Menlo",
    font_size=18,
    line_numbers=True,
    style="monokai",
    image_pad=24,
    line_number_pad=12
)

for filename, code in snippets:
    out_path = os.path.join(OUT_DIR, filename)
    png_data = highlight(code, CLexer(), formatter)
    with open(out_path, "wb") as f:
        f.write(png_data)
    print(f"Generated code snippet card: {out_path}")

print("All syntax-highlighted code cards created successfully!")
