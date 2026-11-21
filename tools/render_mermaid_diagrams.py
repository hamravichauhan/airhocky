import subprocess
import os

DIAGRAMS_DIR = "/Users/ravichauhan/Desktop/COLLEGE/C/PROJ/diagrams"
os.makedirs(DIAGRAMS_DIR, exist_ok=True)

config_path = os.path.join(DIAGRAMS_DIR, "mermaid_config.json")
with open(config_path, "w") as f:
    f.write("""{
  "theme": "base",
  "themeVariables": {
    "fontFamily": "Arial, Helvetica, sans-serif",
    "fontSize": "22px",
    "primaryColor": "#DBEAFE",
    "primaryTextColor": "#0F172A",
    "primaryBorderColor": "#1E3A8A",
    "lineColor": "#1E293B",
    "secondaryColor": "#FEF3C7",
    "tertiaryColor": "#DCFCE7",
    "noteBkgColor": "#FEF08A",
    "noteTextColor": "#713F12",
    "noteBorderColor": "#CA8A04",
    "actorBkg": "#DBEAFE",
    "actorBorder": "#1E3A8A",
    "actorTextColor": "#0F172A",
    "actorLineColor": "#334155",
    "signalColor": "#0F172A",
    "signalTextColor": "#0F172A",
    "labelBoxBkgColor": "#FFFFFF",
    "labelBoxBorderColor": "#1E3A8A",
    "labelTextColor": "#0F172A",
    "loopTextColor": "#0F172A"
  }
}
""")

mmd_arch = """flowchart TD
    subgraph Host ["HOST MACHINE (Game Server & Authoritative Simulator)"]
        direction TB
        subgraph HostInputs ["Host Controls"]
            InputH["Host Input (input.c)\\n• WASD / Mouse Position"]
            AI["AI Controller (ai.c)\\n• 5-State Machine & Adaptive Bias"]
        end
        Phys["Authoritative Physics Engine (game.c)\\n• 6 Sub-steps per Frame • Magnus Spin Curve • Flick Shot Boost"]
        subgraph HostOutputs ["Host Output Subsystems"]
            NetHost["Host POSIX Socket (net.c)\\n• Port 50505 (O_NONBLOCK)"]
            RenderH["Raylib Renderer (render.c)\\n• 4 Visual Themes & Heatmap"]
            AudioH["Procedural Audio DSP (audio.c)\\n• Synthesized Sine/Noise Waves"]
        end
    end

    subgraph Client ["CLIENT MACHINE (Remote Player Node)"]
        direction TB
        InputC["Client Input (input.c)\\n• Captures Local WASD / Mouse"]
        NetClient["Client POSIX Socket (net.c)\\n• Non-Blocking TCP Socket"]
        RenderC["Client Renderer (render.c)\\n• Paints Host World Snapshot"]
    end

    InputH --> Phys
    AI --> Phys
    NetClient -- "1. MSG_INPUT: Paddle Target (tx, ty)" --> NetHost
    NetHost --> Phys
    Phys -- "2. MSG_STATE: Full World Snapshot" --> NetClient
    Phys --> RenderH
    Phys --> AudioH
    NetClient --> RenderC

    style Host fill:#F8FAFC,stroke:#1E3A8A,stroke-width:3px
    style Client fill:#F0F9FF,stroke:#0284C7,stroke-width:3px
    style Phys fill:#FEF08A,stroke:#CA8A04,stroke-width:2.5px
    style NetHost fill:#DBEAFE,stroke:#1E3A8A,stroke-width:2px
    style NetClient fill:#BAE6FD,stroke:#0284C7,stroke-width:2px
    style RenderH fill:#DCFCE7,stroke:#16A34A,stroke-width:2px
    style RenderC fill:#DCFCE7,stroke:#16A34A,stroke-width:2px
    style AudioH fill:#FCE7F3,stroke:#DB2777,stroke-width:2px
    style AI fill:#E0E7FF,stroke:#4338CA,stroke-width:2px
"""

mmd_sync = """sequenceDiagram
    autonumber
    participant Host as Host (Server & Simulator)
    participant Client as Client (Remote Player)

    Note over Host: Sockets Listening on Port 50505
    Client->>Host: TCP Connect Request (inet_pton + connect)
    Host-->>Client: Accept Connection & Send MSG_START
    Note over Host,Client: 3-Second Countdown Begins (Puck Locked at Center)

    loop Every 16.6 ms Frame (60 FPS Game Loop)
        Client->>Client: 1. Read Local Input (input_get_local_target)
        Client->>Host: 2. Send MSG_INPUT (client_tx, client_ty)
        Note over Host: 3. Drain Socket Non-Blockingly (net_poll_input)
        Note over Host: 4. Run Sub-stepped Physics (game_update in 6 steps)
        Note over Host: 5. Detect Paddle Hits, Wall Bounces, Goals
        Host->>Client: 6. Send MSG_STATE (p1, p2, puck, spin, scores, shake)
        Client->>Client: 7. Overwrite Local Variables (net_poll_state)
        Client->>Client: 8. Draw Arena, Paddles & Puck (render_field)
        Host->>Host: 9. Draw Arena & Play Procedural Audio (audio_play)
    end

    alt Goal Scored or Match Finished
        Host->>Client: Broadcast Final Scores & Goal Strobe
        Note over Host,Client: Display Match Statistics & AI Coach Review
    end
"""

mmd_substep = """flowchart LR
    Start(["Physics Start"]) --> SpeedCheck["1. Puck Velocity\nv = sqrt(vx² + vy²)"]
    SpeedCheck --> StepCalc["2. Compute Dynamic Substeps\nsteps = 1 + (v * dt / (0.5 * radius))\nClamp: 1 ≤ steps ≤ 6"]
    StepCalc --> LoopInit["3. Micro-Step Slicing\nsdt = dt / steps (6x CCD)"]
    LoopInit --> Move["4. Advance Position\nx += vx * sdt\ny += vy * sdt"]
    Move --> Resolve["5. Contact & Impulse\n(See Figure 3.3b)"]
    Resolve --> LoopCheck{"More Steps?"}
    LoopCheck -- "Yes" --> Move
    LoopCheck -- "No" --> End(["Physics Resolved\n(Zero Tunneling!)"])

    style Start fill:#E2E8F0,stroke:#475569,stroke-width:2px
    style SpeedCheck fill:#F1F5F9,stroke:#64748B,stroke-width:1.5px
    style StepCalc fill:#FEF3C7,stroke:#D97706,stroke-width:2.5px
    style LoopInit fill:#DBEAFE,stroke:#1E3A8A,stroke-width:2px
    style Move fill:#DBEAFE,stroke:#1E3A8A,stroke-width:2px
    style Resolve fill:#FEF08A,stroke:#CA8A04,stroke-width:2.5px
    style LoopCheck fill:#F8FAFC,stroke:#475569,stroke-width:2px
    style End fill:#DCFCE7,stroke:#16A34A,stroke-width:2.5px
"""

mmd_collision = """flowchart LR
    In["Object Contact"] --> PadCol["Paddle Collision\n• Normal Impulse: -(1+e)*vn\n• Friction -> Angular Spin\n• Flick Boost: +0.35*pad_v"]
    PadCol --> WallCheck{"Boundary Check"}
    WallCheck -- "Goal Slot" --> Goal["Goal Scored!\n• Score Counter++\n• 14px Screen Shake\n• 3-Second Reset"]
    WallCheck -- "Solid Cushion" --> Bounce["Wall Bounce\n• Invert Normal Velocity\n• Emit Sparks & SFX"]
    WallCheck -- "Open Rink" --> Magnus["Magnus Effect\nCurve velocity by spin:\ncurve = spin * 0.0006"]
    Bounce --> Magnus
    Goal --> Out["Substep Complete"]
    Magnus --> Out

    style In fill:#E2E8F0,stroke:#475569,stroke-width:2px
    style PadCol fill:#FEF08A,stroke:#CA8A04,stroke-width:2.5px
    style WallCheck fill:#F8FAFC,stroke:#475569,stroke-width:2px
    style Goal fill:#FEE2E2,stroke:#DC2626,stroke-width:2.5px
    style WallBounce fill:#E0E7FF,stroke:#4338CA,stroke-width:2px
    style Magnus fill:#DCFCE7,stroke:#16A34A,stroke-width:2.5px
    style Out fill:#DCFCE7,stroke:#16A34A,stroke-width:2px
"""

mmd_ai = """stateDiagram-v2
    [*] --> DEFEND : Match Start / Puck in Player Half
    DEFEND --> INTERCEPT : Puck Crosses Midfield & Heading North
    INTERCEPT --> ATTACK : Puck Speed Drops Below Strike Threshold
    ATTACK --> STRIKE : Aligned with Puck & Player Goal Slot
    STRIKE --> DEFEND : Strike Executed / Puck Cleared Back
    INTERCEPT --> DEFEND : Puck Cleared by Defense
    DEFEND --> RESET : Goal Scored / Puck Reset
    RESET --> DEFEND : 3-Second Countdown Completes

    note right of DEFEND
        ADAPTIVE HABIT TRACKING:
        AI shifts resting defense toward
        player's favorite scoring quadrant
    end note

    note right of STRIKE
        STRIKE WINDUP:
        AI paddle steps back 20px before
        explosive forward thrust
    end note
"""

diagram_specs = [
    ("01_high_level_arch.mmd", "01_high_level_arch.png", mmd_arch),
    ("02_sync_flow.mmd", "02_sync_flow.png", mmd_sync),
    ("03a_physics_substep.mmd", "03a_physics_substep.png", mmd_substep),
    ("03b_collision_magnus.mmd", "03b_collision_magnus.png", mmd_collision),
    ("04_ai_state_machine.mmd", "04_ai_state_machine.png", mmd_ai)
]

for mmd_name, png_name, content in diagram_specs:
    mmd_path = os.path.join(DIAGRAMS_DIR, mmd_name)
    png_path = os.path.join(DIAGRAMS_DIR, png_name)
    with open(mmd_path, "w") as f:
        f.write(content)
    cmd = [
        "npx", "-y", "@mermaid-js/mermaid-cli",
        "-i", mmd_path,
        "-o", png_path,
        "-c", config_path,
        "-b", "white",
        "-s", "3"
    ]
    print(f"Rendering {png_name}...")
    subprocess.run(cmd, check=True)
    print(f"Generated: {png_path}")

print("All diagrams rendered with ultra-high quality and large readable text!")
