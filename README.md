# AIR HOCKEY: Real-Time 2D LAN Multiplayer & Systems Simulation Engine

A high-performance, real-time 2D Air Hockey game engine built from scratch in pure **C11** using **raylib** and **low-level sockets (POSIX TCP on Linux/macOS and Winsock2 on Windows)**.

---

## 🚀 Key Highlights & Architectural Features

* **Zero External Asset Dependencies:** Contains no `.png`, `.wav`, or `.mp3` files on disk. 100% of graphics are vector-rendered via OpenGL primitives, and 100% of audio waveforms are synthesized procedurally in RAM at 44.1 kHz.
* **Deterministic 60 FPS Performance:** Input, physics, network synchronization, and graphics all complete within a strict **16.6-millisecond frame budget**.
* **Zero Dynamic Memory Allocation in Game Loop:** All particles, replay frames, and sound buffers utilize pre-allocated static pools to eliminate heap fragmentation and garbage collection stutters.
* **Continuous Collision Detection (CCD):** Implements dynamic micro-stepping (1 to 6 sub-steps per frame) to completely eliminate high-velocity **collision tunneling** at speeds exceeding $1400\text{ px/s}$.
* **Host-Authoritative LAN Multiplayer:** Host acts as the single source of truth over port `50505` with non-blocking sockets (`O_NONBLOCK` / `ioctlsocket`), guaranteeing zero split-screen desync and zero frame-rate stalls.
* **Autonomous Solo Mode Opponent:** Features a human-like 5-state Finite State Machine (`DEFEND`, `INTERCEPT`, `ATTACK`, `STRIKE`, `RESET`) with realistic reaction latency, strike windup, and adaptive spatial habit tracking.
* **Esports Telemetry & Polish:** Instant slow-motion replay buffer (`V` key), predictive cushion bounce raycasting (`CTRL` key), $24 \times 32$ territorial heatmap (`TAB` key), arena power-up drops, and 4 arena themes (`T` key).

---

## 🎮 Playable Game Modes

1. **Solo Mode:** Single player vs. the 5-state adaptive computer opponent (*Easy, Medium, Hard* presets).
2. **Local 2-Player:** Split-keyboard competition on a single laptop/PC.
3. **Host LAN Game:** Host an authoritative game server over local Wi-Fi or Ethernet on port `50505` with automated 4-digit room code beacon broadcast.
4. **Join LAN Game:** Connect directly to a host machine via 4-digit room code auto-discovery or direct IPv4 address.
5. **Time Attack:** 60-second high-score sprint against the clock to score maximum goals.
6. **Tournament Mode:** Multi-match championship series in **Best-of-3 (BO3)** or **Best-of-5 (BO5)** formats.
7. **Bot vs. Bot Demo:** Automated exhibition match showcasing two autonomous bots competing.

---

## 🕹️ Controls Reference

| Control Action | Key / Input | Mode / Context | Engine Response |
| :--- | :--- | :--- | :--- |
| **Navigate Menus** | `W` / `S` or `UP` / `DOWN` | Menu Screens | Cycles highlighted item with audio tick |
| **Select Option** | `ENTER` | Menu Screens | Selects button with smooth alpha screen fade |
| **Return / Back** | `ESC` | Any Screen | Returns to the previous screen or Main Menu |
| **Player 1 Paddle** | `W`, `A`, `S`, `D` | Gameplay | Accelerates paddle; clamped to lower half-court |
| **Player 2 Paddle** | `Arrow Keys` | Local Play | Accelerates paddle; clamped to upper half-court |
| **Flick Shot** | Fast Paddle Swing | Gameplay | Imparts $+0.35 \times \text{paddle\_speed}$ momentum boost |
| **Trajectory Aim Guide** | Hold `LEFT CTRL` | Gameplay | Raycasts 60 forward steps showing bounce angles |
| **Territorial Heatmap** | Press `TAB` | Gameplay | Toggles $24 \times 32$ puck density grid overlay |
| **Instant Slow-Mo Replay** | Press `V` | Gameplay | Replays last 5 seconds in $0.35\times$ slow motion |
| **Switch Arena Theme** | Press `T` | Gameplay | Cycles themes: `Neon` $\rightarrow$ `Retro` $\rightarrow$ `Dark` $\rightarrow$ `Fire` |

---

## 📦 How to Build and Run

The project features a unified cross-platform `Makefile` that automatically detects your operating system and links the appropriate platform libraries.

---

### 🐧 Linux (Fedora, RHEL, CentOS)

1. **Install dependencies:**
   ```bash
   sudo dnf install -y raylib-devel gcc make
   ```
2. **Build and run:**
   ```bash
   make
   ./airhockey
   ```

---

### 🐧 Linux (Ubuntu, Debian, Linux Mint)

1. **Install dependencies:**
   ```bash
   sudo apt update
   sudo apt install -y libraylib-dev gcc make
   ```
2. **Build and run:**
   ```bash
   make
   ./airhockey
   ```

---

### 🪟 Windows (Option 1: WSL — Recommended & Easiest)

Running through **WSL (Windows Subsystem for Linux / Ubuntu)** is the fastest way to run the game on Windows with zero toolchain setup:

1. Open the **Ubuntu / WSL terminal** on your Windows laptop.
2. Install raylib and build tools:
   ```bash
   sudo apt update
   sudo apt install -y libraylib-dev gcc make
   ```
3. Navigate to the project directory and run:
   ```bash
   make
   ./airhockey
   ```

---

### 🪟 Windows (Option 2: Native MinGW / MSYS2 `.exe`)

To compile a native standalone Windows `airhockey.exe`:

1. Open **MSYS2 UCRT64** terminal and install raylib and GCC:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-raylib make
   ```
2. Build the native `.exe`:
   ```bash
   make
   ```
3. Run the game:
   ```bash
   ./airhockey.exe
   ```

---

### 🍏 macOS (Apple Silicon M1/M2/M3 & Intel)

1. **Install raylib via Homebrew:**
   ```bash
   brew install raylib
   ```
2. **Build and run:**
   ```bash
   make
   ./airhockey
   ```

---

## 🌐 Running a Cross-Platform LAN Match (e.g. Fedora $\longleftrightarrow$ Windows)

You can run an authoritative multiplayer match between two laptops on the same local network using the **4-digit Room Code system**:

1. **Connect Both Machines to the Same Network:**
   * Connect both machines to the same Wi-Fi network or your mobile hotspot.

2. **Start the Host (e.g., on Fedora Linux):**
   * Launch `./airhockey`, select **`Host LAN Game`**, and press `ENTER`.
   * The host will display a big glowing **4-digit Room Code** (e.g., `[ 7429 ]`), along with the local IP address.
   * Background UDP beacons automatically broadcast this code across the local subnet.

3. **Connect the Client (e.g., on Windows or macOS):**
   * Launch the game, select **`Join LAN Game`**, and press `ENTER`.
   * **Instant Auto-Discovery:** The screen will automatically display `Room [7429] detected on local Wi-Fi!` — simply press `ENTER` to connect!
   * **Manual Code Entry:** If not auto-detected, simply type the 4-digit code (`7429`) and press `ENTER`.
   * *(Direct IP fallback is also supported: you can type the Host's IP address directly if desired).*

4. **Synchronized Play:**
   * Both screens instantly lock into a synchronized **3-second countdown** (*3... 2... 1... GO!*).
   * **Laptop 1 (Host):** Controls Player 1 (Blue paddle at the bottom).
   * **Laptop 2 (Client):** Controls Player 2 (Red paddle at the top) with zero desynchronization.

---

## 📁 Codebase Directory Structure

```text
├── Makefile                   # Multi-platform build system (Linux, macOS, Windows)
├── README.md                  # Comprehensive build & run instructions
├── Air_Hockey_Presentation.pptx # 16-Slide college viva presentation deck
├── Air_Hockey_Project_Report.docx # Complete formatted academic project report
├── PROJECT_REPORT.md          # Markdown mirrored copy of the project report
├── VIDEO_PRODUCTION_SCRIPT.md # 10-Minute technical video voiceover script
├── diagrams/                  # 5 High-resolution Mermaid architecture flowcharts
├── screenshots/               # 10 Native 60 FPS in-game screen captures
├── video_assets/              # 5 High-speed 60 FPS video clips & 5 code cards
├── src/                       # 18 C11 source and header compilation units
│   ├── main.c                 # Master orchestrator & screen manager
│   ├── game.c / game.h        # Continuous sub-stepping physics & kinematics
│   ├── net.c / net.h          # Cross-platform sockets (POSIX + Winsock2)
│   ├── protocol.h             # 12-byte MSG_INPUT & 56-byte MSG_STATE packets
│   ├── solomode.c / solomode.h# 5-State FSM autonomous opponent with learning
│   ├── audio.c / audio.h      # Procedural PCM audio synthesizer in RAM
│   ├── dynaudio.c / dynaudio.h# Dynamic audio tension & pitch scaling
│   ├── render.c / render.h    # 2D OpenGL rendering & HUD
│   ├── ui.c / ui.h            # Vector UI panels, glowing titles & buttons
│   ├── ui_transition.c / .h   # Alpha cross-fade screen transitions
│   ├── particles.c / .h       # 512-Particle static memory pool
│   ├── powerup.c / .h         # Arena collectible drops (Grow, Slow, Freeze)
│   ├── stats.c / .h           # Match telemetry & airhockey.dat persistence
│   ├── heatmap.c / .h         # 24x32 Territorial density grid with decay
│   ├── trajectory.c / .h      # 60-Step predictive cushion raycasting
│   ├── tournament.c / .h      # Best-of-3 & Best-of-5 series progression
│   ├── themes.c / .h          # 4 Visual color palettes (Neon, Retro, Dark, Fire)
│   └── replay.c / .h          # Circular ring buffer & replay serialization
└── tools/                     # Automated tools for screenshots, clips & docs
```

---

## 📜 Academic Verification
* **Language Standard:** ISO C11 (`-std=c11 -Wall -Wextra -O2`)
* **Memory Safety:** Profiled with **Valgrind** — 0 memory leaks, 0 heap allocations during gameplay loop.
* **Portability:** Verified on Fedora Linux, Ubuntu, macOS (Apple Silicon ARM64 & Intel x86_64), and Windows.
