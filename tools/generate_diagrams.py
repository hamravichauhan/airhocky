import subprocess
import os

DIAGRAMS_DIR = "/Users/ravichauhan/Desktop/COLLEGE/C/PROJ/diagrams"
os.makedirs(DIAGRAMS_DIR, exist_ok=True)

# 1. High-Level Architecture Diagram
dot_arch = """
digraph HighLevelArch {
    graph [rankdir=LR, bgcolor="transparent", fontname="Helvetica", nodesep=0.5, ranksep=0.8];
    node [fontname="Helvetica", shape=box, style="filled,rounded", penwidth=1.5, margin="0.2,0.15", fontsize=11];
    edge [fontname="Helvetica", fontsize=10, color="#334155", penwidth=1.5];

    subgraph cluster_client {
        label = "CLIENT MACHINE";
        fontname = "Helvetica-Bold";
        fontsize = 13;
        color = "#0284C7";
        penwidth = 2.0;
        style = "filled,rounded";
        fillcolor = "#F0F9FF";

        c_input [label="Client Input\\n(input.c)\\n• Keyboard WASD\\n• Mouse Position", fillcolor="#BAE6FD", color="#0284C7"];
        c_net   [label="Client Socket\\n(net.c)\\n• Non-blocking TCP\\n• Port 50505", fillcolor="#BAE6FD", color="#0284C7"];
        c_rend  [label="Client Renderer\\n(render.c)\\n• Raylib OpenGL\\n• 60 FPS Display", fillcolor="#BAE6FD", color="#0284C7"];

        c_input -> c_net [label="Target (tx, ty)"];
        c_net -> c_rend [label="Snapshot"];
    }

    subgraph cluster_host {
        label = "HOST MACHINE (SERVER & SIMULATOR)";
        fontname = "Helvetica-Bold";
        fontsize = 13;
        color = "#1E3A8A";
        penwidth = 2.0;
        style = "filled,rounded";
        fillcolor = "#F8FAFC";

        h_input [label="Host Input\\n(input.c)\\n• Player 1 Controls", fillcolor="#DBEAFE", color="#1E3A8A"];
        h_net   [label="Host Socket\\n(net.c)\\n• O_NONBLOCK\\n• Authoritative", fillcolor="#DBEAFE", color="#1E3A8A"];
        h_phys  [label="Physics Engine\\n(game.c)\\n• 6-Step Substepping\\n• Magnus Spin Curve\\n• Flick Shot Boost", fillcolor="#FEF08A", color="#CA8A04"];
        h_ai    [label="AI Engine\\n(ai.c)\\n• Finite State Machine\\n• Adaptive Habit Bias", fillcolor="#E0E7FF", color="#4338CA"];
        h_rend  [label="Renderer & FX\\n(render.c, particles.c)\\n• 4 Color Themes\\n• 512-Particle Pool", fillcolor="#DCFCE7", color="#16A34A"];
        h_audio [label="Audio DSP Synthesizer\\n(audio.c)\\n• Procedural Waves\\n• Dynamic Tension", fillcolor="#FCE7F3", color="#DB2777"];

        h_input -> h_phys;
        h_ai -> h_phys;
        h_net -> h_phys [label="Client Paddle Target"];
        h_phys -> h_rend [label="Positions & Events"];
        h_phys -> h_audio [label="Impulse / Goal SFX"];
        h_phys -> h_net [label="World Snapshot"];
    }

    c_net -> h_net [label="MSG_INPUT\\n(Paddle Target)", color="#0284C7", style="bold"];
    h_net -> c_net [label="MSG_STATE\\n(Full World Snapshot)", color="#1E3A8A", style="bold"];
}
"""

# 2. Multiplayer Synchronization Flow
dot_sync = """
digraph SyncProtocol {
    graph [rankdir=TB, bgcolor="transparent", fontname="Helvetica", nodesep=0.4, ranksep=0.5];
    node [fontname="Helvetica", shape=box, style="filled,rounded", penwidth=1.5, margin="0.2,0.12", fontsize=11];
    edge [fontname="Helvetica", fontsize=10, color="#475569", penwidth=1.4];

    start [label="Connection Handshake\\nClient connects via TCP -> Host accepts\\nBoth enter 3-second countdown (Puck locked)", fillcolor="#E2E8F0", color="#475569"];
    
    frame_loop [label="60 FPS Master Game Loop (Every 16.6 ms)", shape=ellipse, fillcolor="#FEF3C7", color="#D97706", fontname="Helvetica-Bold", fontsize=12];
    
    step1 [label="1. Client Input Capture\\ninput_get_local_target() reads WASD / Mouse coordinates", fillcolor="#BAE6FD", color="#0284C7"];
    step2 [label="2. Client Transmits MSG_INPUT\\nSends Packet containing (client_tx, client_ty) via send_all()", fillcolor="#BAE6FD", color="#0284C7"];
    step3 [label="3. Host Drains Socket (Non-Blocking)\\nnet_poll_input() receives latest client paddle target", fillcolor="#DBEAFE", color="#1E3A8A"];
    step4 [label="4. Host Simulates Physics (game_update)\\nExecutes up to 6 micro-steps of collision, spin, and velocity clamping", fillcolor="#FEF08A", color="#CA8A04"];
    step5 [label="5. Host Broadcasts MSG_STATE\\nSerializes p1, p2, puck (x, y, vx, vy, spin), scores, shake into Packet", fillcolor="#DBEAFE", color="#1E3A8A"];
    step6 [label="6. Client Unpacks Snapshot\\nnet_poll_state() overwrites local coordinates with Host truth", fillcolor="#DCFCE7", color="#16A34A"];
    step7 [label="7. Both Draw Display (render_field)\\nBoth machines paint identical frames on their screens (Zero Desync!)", fillcolor="#DCFCE7", color="#16A34A"];

    start -> frame_loop;
    frame_loop -> step1;
    step1 -> step2;
    step2 -> step3;
    step3 -> step4;
    step4 -> step5;
    step5 -> step6;
    step6 -> step7;
    step7 -> frame_loop [label="Next Frame", style="dashed"];
}
"""

# 3. Physics & Continuous Collision Detection Pipeline
dot_phys = """
digraph PhysicsPipeline {
    graph [rankdir=TB, bgcolor="transparent", fontname="Helvetica", nodesep=0.4, ranksep=0.5];
    node [fontname="Helvetica", shape=box, style="filled,rounded", penwidth=1.5, margin="0.2,0.12", fontsize=11];
    edge [fontname="Helvetica", fontsize=10, color="#334155", penwidth=1.4];

    start [label="Frame Physics Start (dt ≤ 0.05s)", fillcolor="#E2E8F0", color="#475569", fontname="Helvetica-Bold"];
    calc_speed [label="Calculate Current Puck Velocity\\nspeed = sqrt(vx² + vy²)", fillcolor="#F1F5F9", color="#64748B"];
    calc_steps [label="Determine Dynamic Substep Count\\nsteps = 1 + (speed * dt / (PUCK_R * 0.5))\\nClamp: 1 ≤ steps ≤ 6", fillcolor="#FEF3C7", color="#D97706"];
    
    substep_start [label="Substep Integration (sdt = dt / steps)\\npuck.x += puck.vx * sdt\\npuck.y += puck.vy * sdt", fillcolor="#DBEAFE", color="#1E3A8A"];
    
    collide_paddles [label="Collide With Paddles 1 & 2\\n• Positional Overlap Correction\\n• Normal Impulse: j = -(1 + e) * vn\\n• Tangential Friction -> Imparts Spin\\n• Flick Boost: +0.35 * paddle_speed", fillcolor="#FEF08A", color="#CA8A04"];
    
    check_wall [label="Check Arena Cushion Boundaries", shape=diamond, fillcolor="#F8FAFC", color="#475569"];
    
    goal_scored [label="Goal Opening Hit!\\n• Increment Score\\n• Trigger Shake (14px) & Flash\\n• Reset Puck to Center (3s timer)", fillcolor="#FEE2E2", color="#DC2626"];
    wall_bounce [label="Solid Wall Hit\\n• Invert Normal Velocity (vx or vy)\\n• Emit Impact Sparks & Play SFX", fillcolor="#E0E7FF", color="#4338CA"];
    
    magnus [label="Apply Magnus Effect (Curving Puck)\\ncurve = spin * 0.0006\\nRotate velocity vector perpendicular to flight", fillcolor="#DCFCE7", color="#16A34A"];
    
    check_substeps [label="More Substeps Remaining?", shape=diamond, fillcolor="#F8FAFC", color="#475569"];
    end [label="Frame Physics Resolved", fillcolor="#E2E8F0", color="#475569", fontname="Helvetica-Bold"];

    start -> calc_speed;
    calc_speed -> calc_steps;
    calc_steps -> substep_start;
    substep_start -> collide_paddles;
    collide_paddles -> check_wall;
    check_wall -> goal_scored [label="In Goal Slot"];
    check_wall -> wall_bounce [label="Cushion Boundary"];
    check_wall -> magnus [label="Open Rink"];
    wall_bounce -> magnus;
    magnus -> check_substeps;
    check_substeps -> substep_start [label="Yes (Loop)", style="bold", color="#1E3A8A"];
    check_substeps -> end [label="No (Done)"];
}
"""

# 4. AI State Machine
dot_ai = """
digraph AIStateMachine {
    graph [rankdir=LR, bgcolor="transparent", fontname="Helvetica", nodesep=0.6, ranksep=0.7];
    node [fontname="Helvetica", shape=box, style="filled,rounded", penwidth=1.5, margin="0.25,0.15", fontsize=11];
    edge [fontname="Helvetica", fontsize=10, color="#475569", penwidth=1.4];

    defend [label="DEFEND STATE\\n• Stays in defensive arc\\n• Adjusts resting position based\\n  on player goal history (Adaptive)", fillcolor="#DBEAFE", color="#1E3A8A", fontname="Helvetica-Bold"];
    intercept [label="INTERCEPT STATE\\n• Puck moving toward AI court\\n• Moves to predicted bounce position", fillcolor="#FEF3C7", color="#D97706", fontname="Helvetica-Bold"];
    attack [label="ATTACK STATE\\n• Puck is slow / stationary\\n• Closes in for offensive strike", fillcolor="#DCFCE7", color="#16A34A", fontname="Helvetica-Bold"];
    strike [label="STRIKE STATE\\n• Windup step backwards\\n• Explosive forward thrust at goal angle", fillcolor="#FEE2E2", color="#DC2626", fontname="Helvetica-Bold"];
    reset [label="RESET STATE\\n• Goal scored\\n• Return to center home post", fillcolor="#F1F5F9", color="#64748B", fontname="Helvetica-Bold"];

    defend -> intercept [label="Puck crosses midfield\\n& vy < 0", color="#1E3A8A"];
    intercept -> attack [label="Puck near AI &\\nspeed < strike_threshold", color="#D97706"];
    attack -> strike [label="Aligned with puck\\n& opponent goal", color="#16A34A"];
    strike -> defend [label="Puck hit / cleared\\nback to player half", color="#475569"];
    intercept -> defend [label="Puck clears away", color="#64748B"];
    defend -> reset [label="Goal scored", color="#DC2626"];
    reset -> defend [label="Countdown finished", color="#1E3A8A"];
}
"""

diagrams = [
    ("01_high_level_arch.dot", "01_high_level_arch.png", dot_arch),
    ("02_sync_flow.dot", "02_sync_flow.png", dot_sync),
    ("03_physics_pipeline.dot", "03_physics_pipeline.png", dot_phys),
    ("04_ai_state_machine.dot", "04_ai_state_machine.png", dot_ai),
]

for dot_file, png_file, content in diagrams:
    dot_path = os.path.join(DIAGRAMS_DIR, dot_file)
    png_path = os.path.join(DIAGRAMS_DIR, png_file)
    with open(dot_path, "w") as f:
        f.write(content)
    subprocess.run(["/opt/homebrew/bin/dot", "-Tpng", "-Gdpi=200", dot_path, "-o", png_path], check=True)
    print(f"Generated diagram: {png_path}")
