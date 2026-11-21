# 10-Minute Technical Video Production Script & Storyboard
## "Building an Air Hockey Simulation Engine in Pure C & Raylib"

---

## Executive Production Overview

* **Video Target Length:** 10:00 Minutes (600 Seconds)
* **Target Audience:** College professors, external examiners, software engineers, and MCA evaluation panels.
* **Tone:** Confident, structured, enthusiastic, and technically rigorous.
* **Core Message:** *"I chose to build an Air Hockey simulation engine in pure C to move beyond generic CRUD web apps and master low-level systems programming: raw non-blocking POSIX TCP networking, continuous collision detection (CCD) physics, autonomous AI state machines, and mathematical procedural audio DSP."*

---

## Video Asset Directory Index

All media assets referenced in this script are organized and pre-rendered in your workspace:

* **Gameplay Video Clips (60 FPS .mp4):** `video_assets/clips/`
  * `01_fast_rally_goal.mp4` (High-speed rally, power-up sparks, screen shake, goal celebration)
  * `02_trajectory_flick.mp4` (Predictive trajectory raycasting & flick shot physics)
  * `03_heatmap_active.mp4` (Real-time $24 \times 32$ territorial heatmap density grid)
  * `04_theme_showcase.mp4` (Dynamic cycling between Neon, Retro, Dark, and Fire palettes)
  * `05_slowmo_replay.mp4` (Instant slow-motion replay banner playback)
* **Architecture Flowcharts (Mermaid Diagrams):** `diagrams/`
  * `01_high_level_arch.png` (High-Level Host vs Client System Architecture)
  * `02_sync_flow.png` (Network Handshake & 60 FPS Synchronization Sequence)
  * `03a_physics_substep.png` (Dynamic Sub-Step Time Slicing & Loop Pipeline)
  * `03b_collision_magnus.png` (Contact Impulse, Wall Cushions & Magnus Spin Curvature)
  * `04_ai_state_machine.png` (Autonomous AI State Machine & Habit Adaptation)
* **In-Game Screen Captures (Native Raylib):** `screenshots/`
  * `01_main_menu.png` (Main Menu & Navigation Interface)
  * `02_gameplay_neon.png` (Real-time match view with paddles, puck, and spark FX)
  * `03_trajectory_prediction.png` (Predictive reflection raycasting view)
  * `04_heatmap_overlay.png` (Heat density grid view)
  * `05_match_stats.png` (Post-match telemetry & records screen)
  * `06_time_attack.png` (Time Attack 60-second high-score results)
  * `07_settings_menu.png` (Configuration sliders & toggles UI)
  * `08_tournament_mode.png` (Best-of-3 / Best-of-5 tournament selector)
  * `09_lan_join.png` (LAN multiplayer IP connection interface)

---

## Detailed 10-Minute Storyboard & Script Breakdown

---

### [00:00 - 01:15] SECTION 1: The Hook & "Why I Chose This Project"
* **Duration:** 1 Minute 15 Seconds
* **On-Screen Visuals:**
  * **0:00 - 0:25:** Cut immediately to `01_fast_rally_goal.mp4` (Full screen, 60 FPS gameplay, sparks flying, fast puck rally ending in a goal explosion).
  * **0:25 - 0:50:** Title slide + `01_main_menu.png` showcasing the 10 game modes.
  * **0:50 - 1:15:** Side-by-side comparison slide: Generic Web Apps (MERN/CRUD) vs. Low-Level Systems Programming (C11, Raylib, POSIX Sockets, Math Physics).

> **Spoken Voiceover Script:**
> *"Hello everyone! Today, I’m excited to walk you through the engineering and architecture of my project: a **real-time 2D Air Hockey game and simulation engine built in pure C11 using raylib and POSIX TCP sockets**.*
> 
> *When deciding on our MCA final project, almost everyone around us was building standard CRUD web applications—e-commerce portals, library management systems, or basic dashboards. While those are practical, I wanted a challenge that would truly push my understanding of core Computer Science: **operating systems, non-blocking network programming, numerical physics integration, and digital signal processing**.*
> 
> *In this 10-minute walkthrough, I will show you how we built this entire 6,300-line engine from scratch—without relying on heavy game engines like Unity, without any physics libraries, and without even a single audio file on disk. Let's dive in!"*

---

### [01:15 - 02:45] SECTION 2: System Architecture & Host-Authoritative Model
* **Duration:** 1 Minute 30 Seconds
* **On-Screen Visuals:**
  * **1:15 - 1:55:** Full screen display of **Figure 3.1** (`diagrams/01_high_level_arch.png`). Highlight the Host Machine box and Client Machine box.
  * **1:55 - 2:45:** Zoom in on **Figure 3.2** (`diagrams/02_sync_flow.png`). Animated timeline showing the TCP handshake, 3-second countdown lock, and the continuous exchange of `MSG_INPUT` and `MSG_STATE`.

> **Spoken Voiceover Script:**
> *"Let's start with the high-level architecture. In any real-time multiplayer physics game, the biggest challenge is **desynchronization**. If two computers calculate physics independently, slight floating-point differences will cause the puck to diverge, meaning one player sees a goal while the other sees a save.*
> 
> *To solve this, we designed a **Host-Authoritative Client-Server architecture**.*
> 
> *As you can see in Figure 3.1, one machine acts as the Host server, and the other connects as the Client over LAN or Wi-Fi on port 50505.*
> 
> *The Host is the single source of truth. It runs the physics simulation across 6 micro-steps, simulates AI, checks collisions, and tracks scores. The Client is essentially a thin input/output terminal. Every frame, the client captures user mouse or keyboard input and sends a 12-byte `MSG_INPUT` packet to the host. The host absorbs that target, advances the physical universe, and broadcasts a 56-byte `MSG_STATE` snapshot back to the client.*
> 
> *Because both machines operate on non-blocking sockets, packet delivery on a local network takes less than 3 milliseconds, achieving **zero desynchronization and a flawless 60 frames per second**."*

---

### [02:45 - 04:30] SECTION 3: Deep Dive into Networking Code (`net.c` & `protocol.h`)
* **Duration:** 1 Minute 45 Seconds
* **On-Screen Visuals:**
  * **2:45 - 3:20:** Show code of `src/protocol.h` highlighting the `Packet` struct and message types.
  * **3:20 - 3:55:** Show `src/net.c` (lines 30–36) highlighting `fcntl(fd, F_SETFL, O_NONBLOCK)`.
  * **3:55 - 4:15:** Show `src/net.c` (lines 39–83) showing the `send_all()` and `recv_all()` loops.
  * **4:15 - 4:30:** Cut to `screenshots/09_lan_join.png` showing the client typing an IP address to join.

> **Spoken Voiceover Script:**
> *"Now let's look at the code that makes this possible. Inside `protocol.h`, we defined a binary communication protocol. Rather than transmitting heavy JSON or text strings, we use a compact C struct called `Packet`. This packet holds exact coordinate floats for the paddles, puck velocity, angular spin, scores, and screen shake intensity.*
> 
> *In `net.c`, you'll notice our `set_nonblocking()` helper function. By default, socket calls like `recv()` block the CPU thread until data arrives. If someone’s Wi-Fi hiccuped for even 50 milliseconds, a standard game would freeze. By using `fcntl()` with the `O_NONBLOCK` flag, `recv()` returns immediately if no new packet is in the OS buffer. The game loop never stalls.*
> 
> *Furthermore, because TCP is a stream protocol that can fragment packets, we wrote custom `send_all()` and `recv_all()` functions. These loops handle partial OS socket buffers and catch `EAGAIN` or `EWOULDBLOCK` errors, guaranteeing that every packet is received completely without memory corruption."*

---

### [04:30 - 06:15] SECTION 4: Physics Engine, Sub-Stepping & Magnus Effect (`game.c`)
* **Duration:** 1 Minute 45 Seconds
* **On-Screen Visuals:**
  * **4:30 - 5:05:** Full screen display of **Figure 3.3a** (`diagrams/03a_physics_substep.png`).
  * **5:05 - 5:40:** Display of **Figure 3.3b** (`diagrams/03b_collision_magnus.png`).
  * **5:40 - 6:15:** Cut to `02_trajectory_flick.mp4` (Video clip showing puck curving in flight with yellow bounce prediction dots).

> **Spoken Voiceover Script:**
> *"Next is the physical simulation in `game.c`. In basic games, physics is updated once per frame. But our puck can travel up to 1400 pixels per second! At 60 FPS, that’s 23 pixels in a single frame. A fast puck could literally teleport through a thin wall or paddle—a classic bug known as 'tunneling'.*
> 
> *To solve this, we implemented **Continuous Collision Detection using dynamic sub-stepping**, as shown in Figure 3.3a. Every frame, we calculate:
> `steps = 1 + (speed * dt / (radius * 0.5))`
> clamped up to 6 micro-steps. We slice time into smaller intervals (`sdt = dt / steps`), ensuring the puck never travels more than half its radius in any single calculation.*
> 
> *We also added realistic sports mechanics:
> First, the **Flick Shot**: fast paddle swings transfer extra momentum (`+0.35 * paddle_speed`) directly to the puck.
> Second, the **Magnus Effect**: when a paddle glances the puck at an angle, friction imparts rotational spin. We calculate a perpendicular acceleration vector:
> `curve = spin * 0.0006`,
> causing the puck to curve gracefully in flight just like in real air hockey! In `trajectory.c`, we raycast 60 forward steps into the future, mirroring bounce angles off rink cushions so players can aim trick bank shots."*

---

### [06:15 - 07:45] SECTION 5: Solo Mode Opponent Engine (`ai.c`, `stats.c`)
* **Duration:** 1 Minute 30 Seconds
* **On-Screen Visuals:**
  * **6:15 - 6:50:** Display of **Figure 3.4** (`diagrams/04_ai_state_machine.png`).
  * **6:50 - 7:20:** Show `src/ai.c` adaptive habit tracking code (`ai_record_player_goal`).
  * **7:20 - 7:45:** Cut to `screenshots/05_match_stats.png` showing the post-game telemetry and records screen.

> **Spoken Voiceover Script:**
> *"In Solo Mode, we didn't want a dumb computer opponent that simply glued its X-coordinate to the puck. In `ai.c`, we designed a **Finite State Machine** with five distinct states:
> `DEFEND`, `INTERCEPT`, `ATTACK`, `STRIKE`, and `RESET`.*
> 
> *When defending, the AI stays in a protective arc. When the puck slows down near midfield, the AI transitions into attack mode, executes a realistic 20-pixel windup step backwards, and drives forward with an explosive strike.*
> 
> *Even cooler is our **Adaptive Learning algorithm**: the AI monitors which third of the goal you score on—left, center, or right. If it notices you repeatedly score on its left corner, it shifts its resting defensive stance to the left, actively forcing you to change your tactics!*
> 
> *And when the match ends, `stats.c` calculates your hit-to-goal conversion ratio, rally longevity, and top shot speeds, persisting lifetime records to disk so you can track your skill progression over time."*

---

### [07:45 - 08:50] SECTION 6: Procedural Audio DSP & Game Feel Juice
* **Duration:** 1 Minute 05 Seconds
* **On-Screen Visuals:**
  * **7:45 - 8:15:** Show `src/audio.c` (`make_wave`, sine wave generation, exponential decay).
  * **8:15 - 8:35:** Cut to `04_theme_showcase.mp4` (Showing Neon, Retro, Dark, and Fire palettes cycling).
  * **8:35 - 8:50:** Cut to `03_heatmap_active.mp4` and `05_slowmo_replay.mp4`.

> **Spoken Voiceover Script:**
> *"One of the aspects I'm proudest of is our **Procedural Audio Synthesizer** in `audio.c`.
> If you inspect our project directory, you will notice something surprising: **there is not a single .wav or .mp3 file**.*
> 
> *Instead, we write raw digital audio buffers directly into memory at 44,100 Hz. Paddle hits, wall thuds, countdown beeps, and fanfare are synthesized mathematically using sine waves, white noise, and exponential decay envelopes:
> `S(t) = A * sin(2 * pi * f * t) * exp(-t * decay)`.*
> 
> *Collision velocities dynamically scale the volume and pitch of the sound!*
> 
> *We paired this with maximum game feel—what game developers call 'juice': a 512-particle pool for sparks and confetti, camera screen shake on goals, a 24x32 live territorial heatmap, and 4 customizable visual themes."*

---

### [08:50 - 10:00] SECTION 7: Conclusion, Testing & Viva Defense Summary
* **Duration:** 1 Minute 10 Seconds
* **On-Screen Visuals:**
  * **8:50 - 9:20:** Show `screenshots/05_match_stats.png` and `screenshots/08_tournament_mode.png`.
  * **9:20 - 9:45:** Slide listing Project Achievements: 6,320 lines of C11, 22 module pairs, zero memory leaks, cross-platform macOS/Linux compilation via Makefile.
  * **9:45 - 10:00:** Title card with GitHub/project link, team member acknowledgments, and closing thanks.

> **Spoken Voiceover Script:**
> *"To conclude, this project proved to be an invaluable hands-on learning experience. Over 6,320 lines of modular C11 code, we successfully demonstrated:
> 1. Real-time distributed systems using non-blocking TCP sockets.
> 2. Numerical kinematics and collision sub-stepping.
> 3. Autonomous state machine AI with habit adaptation.
> 4. Mathematical digital signal processing in C.
> 
> The game compiles cleanly with standard GCC and Clang on macOS and Linux with zero external asset dependencies.
> 
> Thank you so much for watching! We welcome any questions from the panel regarding socket implementation, physics integration, or data architecture."*

---
*Script and Storyboard created for MCA Final Project Video Presentation.*
