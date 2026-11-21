import os
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN, MSO_ANCHOR
from pptx.enum.shapes import MSO_SHAPE

PPT_PATH = "/Users/ravichauhan/Desktop/COLLEGE/C/PROJ/Air_Hockey_Presentation.pptx"

def create_presentation():
    prs = Presentation()
    # 16:9 Widescreen dimensions
    prs.slide_width = Inches(13.333)
    prs.slide_height = Inches(7.5)
    blank_layout = prs.slide_layouts[6]

    # Color Palette
    C_NAVY = RGBColor(15, 23, 42)         # #0F172A Deep Navy
    C_BLUE = RGBColor(30, 58, 138)        # #1E3A8A Primary Accent
    C_SKY = RGBColor(2, 132, 199)         # #0284C7 Bright Accent
    C_CYAN = RGBColor(56, 189, 248)       # #38BDF8 Bright Cyan
    C_GOLD = RGBColor(217, 119, 6)        # #D97706 Warm Gold
    C_TEXT_DARK = RGBColor(30, 41, 59)    # #1E293B
    C_TEXT_MUTED = RGBColor(100, 116, 139)# #64748B
    C_BG_CARD = RGBColor(241, 245, 249)   # #F1F5F9 Soft Card
    C_BG_WHITE = RGBColor(255, 255, 255)
    C_BORDER = RGBColor(226, 232, 240)    # #E2E8F0

    def add_header(slide, title_text, category_text="MCA PROJECT PRESENTATION"):
        # Category Tag
        cat_box = slide.shapes.add_textbox(Inches(0.8), Inches(0.4), Inches(11.5), Inches(0.35))
        tf_cat = cat_box.text_frame
        tf_cat.word_wrap = True
        p_cat = tf_cat.paragraphs[0]
        p_cat.text = category_text.upper()
        p_cat.font.size = Pt(11)
        p_cat.font.bold = True
        p_cat.font.color.rgb = C_SKY

        # Main Title
        title_box = slide.shapes.add_textbox(Inches(0.8), Inches(0.7), Inches(11.5), Inches(0.8))
        tf_title = title_box.text_frame
        tf_title.word_wrap = True
        p_title = tf_title.paragraphs[0]
        p_title.text = title_text
        p_title.font.size = Pt(26)
        p_title.font.bold = True
        p_title.font.color.rgb = C_NAVY

        # Accent Line
        line = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0.8), Inches(1.5), Inches(11.7), Inches(0.04))
        line.fill.solid()
        line.fill.fore_color.rgb = C_SKY
        line.line.color.rgb = C_SKY

    def add_footer(slide, current_slide, total_slides=16):
        box = slide.shapes.add_textbox(Inches(0.8), Inches(6.9), Inches(11.7), Inches(0.4))
        tf = box.text_frame
        p = tf.paragraphs[0]
        r1 = p.add_run()
        r1.text = "MCA Project Presentation  •  Air Hockey C11 Multiplayer Engine"
        r1.font.size = Pt(10)
        r1.font.color.rgb = C_TEXT_MUTED
        
        r2 = p.add_run()
        r2.text = f"                                                                                                  Slide {current_slide} of {total_slides}"
        r2.font.size = Pt(10)
        r2.font.color.rgb = C_TEXT_MUTED

    # ---------------- SLIDE 1: TITLE SLIDE ----------------
    slide1 = prs.slides.add_slide(blank_layout)
    bg1 = slide1.shapes.add_shape(MSO_SHAPE.RECTANGLE, 0, 0, Inches(13.333), Inches(7.5))
    bg1.fill.solid()
    bg1.fill.fore_color.rgb = C_NAVY
    bg1.line.color.rgb = C_NAVY

    # Decorative Border
    d_rect = slide1.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(0.8), Inches(0.8), Inches(11.733), Inches(5.9))
    d_rect.fill.background()
    d_rect.line.color.rgb = RGBColor(51, 65, 85)
    d_rect.line.width = Pt(1.5)

    tb1 = slide1.shapes.add_textbox(Inches(1.2), Inches(1.3), Inches(11.0), Inches(2.2))
    tf1 = tb1.text_frame
    tf1.word_wrap = True

    p_badge = tf1.paragraphs[0]
    p_badge.text = "DEPARTMENT OF COMPUTER APPLICATIONS (MCA)  •  FINAL PROJECT DEFENSE"
    p_badge.font.size = Pt(12)
    p_badge.font.bold = True
    p_badge.font.color.rgb = C_CYAN

    p1 = tf1.add_paragraph()
    p1.text = "AIR HOCKEY"
    p1.font.size = Pt(44)
    p1.font.bold = True
    p1.font.color.rgb = RGBColor(255, 255, 255)

    p_sub = tf1.add_paragraph()
    p_sub.text = "Real-Time 2D LAN Multiplayer & Systems Simulation Engine in Pure C11"
    p_sub.font.size = Pt(18)
    p_sub.font.color.rgb = RGBColor(148, 163, 184)
    p_sub.space_before = Pt(8)

    # Info Cards
    card1 = slide1.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(1.2), Inches(4.0), Inches(5.2), Inches(2.2))
    card1.fill.solid()
    card1.fill.fore_color.rgb = RGBColor(30, 41, 59)
    card1.line.color.rgb = RGBColor(51, 65, 85)
    tf_c1 = card1.text_frame
    tf_c1.word_wrap = True
    p_c1 = tf_c1.paragraphs[0]
    p_c1.text = "STUDENT TEAM CONTRIBUTION"
    p_c1.font.size = Pt(11)
    p_c1.font.bold = True
    p_c1.font.color.rgb = C_CYAN

    members = [
        "MCA-2026-01: Team Lead (Network Architecture & Sockets)",
        "MCA-2026-02: Physics & Continuous Collision Engine",
        "MCA-2026-03: Graphics & Procedural Audio DSP Synthesis",
        "MCA-2026-04: Solo Mode Opponent Engine & Telemetry"
    ]
    for m in members:
        p = tf_c1.add_paragraph()
        r = p.add_run()
        r.text = "•  " + m
        r.font.size = Pt(11)
        r.font.color.rgb = RGBColor(226, 232, 240)

    card2 = slide1.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(6.8), Inches(4.0), Inches(5.3), Inches(2.2))
    card2.fill.solid()
    card2.fill.fore_color.rgb = RGBColor(30, 41, 59)
    card2.line.color.rgb = RGBColor(51, 65, 85)
    tf_c2 = card2.text_frame
    tf_c2.word_wrap = True
    p_c2 = tf_c2.paragraphs[0]
    p_c2.text = "PROJECT METADATA & GUIDANCE"
    p_c2.font.size = Pt(11)
    p_c2.font.bold = True
    p_c2.font.color.rgb = C_CYAN

    meta = [
        "Project Type: Real-Time Systems Programming in C11",
        "Faculty Guide: [Guide / Professor Name]",
        "Academic Year: 2026 – 2027",
        "Tech Stack: Pure C11, POSIX / Winsock Sockets, raylib"
    ]
    for me in meta:
        p = tf_c2.add_paragraph()
        r = p.add_run()
        r.text = "•  " + me
        r.font.size = Pt(11)
        r.font.color.rgb = RGBColor(226, 232, 240)

    slide1.notes_slide.notes_text_frame.text = (
        "Good morning, respected members of the evaluation panel. Today, our team is presenting our MCA project: "
        "a real-time 2D Air Hockey multiplayer simulation engine written from scratch in pure C11. We look forward "
        "to walking you through our architecture, features, and engineering solutions."
    )

    # ---------------- HELPER: TWO-COLUMN CARD SLIDE ----------------
    def add_two_column_slide(slide_num, title, category, left_title, left_items, right_title, right_items, notes_text=""):
        slide = prs.slides.add_slide(blank_layout)
        add_header(slide, title, category)
        add_footer(slide, slide_num)

        # Left Card
        card_l = slide.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(0.8), Inches(1.8), Inches(5.6), Inches(4.8))
        card_l.fill.solid()
        card_l.fill.fore_color.rgb = C_BG_CARD
        card_l.line.color.rgb = C_BORDER
        tf_l = card_l.text_frame
        tf_l.word_wrap = True
        p_lt = tf_l.paragraphs[0]
        p_lt.text = left_title
        p_lt.font.size = Pt(16)
        p_lt.font.bold = True
        p_lt.font.color.rgb = C_BLUE

        for b_title, b_desc in left_items:
            p = tf_l.add_paragraph()
            p.space_before = Pt(12)
            r1 = p.add_run()
            r1.text = "•  " + b_title + ": "
            r1.font.bold = True
            r1.font.size = Pt(13)
            r1.font.color.rgb = C_TEXT_DARK
            r2 = p.add_run()
            r2.text = b_desc
            r2.font.size = Pt(13)
            r2.font.color.rgb = C_TEXT_DARK

        # Right Card
        card_r = slide.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(6.8), Inches(1.8), Inches(5.7), Inches(4.8))
        card_r.fill.solid()
        card_r.fill.fore_color.rgb = C_BG_CARD
        card_r.line.color.rgb = C_BORDER
        tf_r = card_r.text_frame
        tf_r.word_wrap = True
        p_rt = tf_r.paragraphs[0]
        p_rt.text = right_title
        p_rt.font.size = Pt(16)
        p_rt.font.bold = True
        p_rt.font.color.rgb = C_BLUE

        for b_title, b_desc in right_items:
            p = tf_r.add_paragraph()
            p.space_before = Pt(12)
            r1 = p.add_run()
            r1.text = "•  " + b_title + ": "
            r1.font.bold = True
            r1.font.size = Pt(13)
            r1.font.color.rgb = C_TEXT_DARK
            r2 = p.add_run()
            r2.text = b_desc
            r2.font.size = Pt(13)
            r2.font.color.rgb = C_TEXT_DARK

        slide.notes_slide.notes_text_frame.text = notes_text
        return slide

    # ---------------- SLIDE 2: BRIEF ABOUT OUR PROJECT ----------------
    add_two_column_slide(
        2,
        "Brief About Our Project",
        "Project Introduction & Motivation",
        "What Is This Project?",
        [
            ("Real-Time Interactive Game", "A fast, responsive 2D Air Hockey game operating at a strict 60 FPS update rate."),
            ("Low-Level C11 Implementation", "Built from scratch close to the operating system using C11 and POSIX/Winsock sockets."),
            ("True Multiplayer Capabilities", "Features cross-machine LAN play over local networks as well as single-player solo modes."),
            ("Integrated Physics & Audio", "Features Newtonian elastic bounces, curving spin mechanics, and procedural sound generated in RAM.")
        ],
        "Why Did We Choose This Project?",
        [
            ("The Trend in Academic Projects", "Over 85% of university projects are standard CRUD web applications that hide core computer science concepts."),
            ("Mastering Systems Concepts", "We wanted to work directly with memory buffers, non-blocking network I/O, and continuous physics loops."),
            ("Deterministic Performance", "No garbage collection pauses (unlike Java or Python) — ensures smooth real-time physical simulation."),
            ("High Technical Merit", "Combines operating systems, networking, physics kinematics, and digital signal processing in one cohesive project.")
        ],
        "Instead of building a typical web application, we chose a systems engineering challenge. We wanted to understand low-level programming: how network packets travel between computers, how physics simulations run at 60 FPS, and how memory is managed without a garbage collector."
    )

    # ---------------- SLIDE 3: PROJECT VISION & PHILOSOPHY ----------------
    add_two_column_slide(
        3,
        "Project Vision & Core Principles",
        "Architectural Standards",
        "Core Engineering Constraints",
        [
            ("Pure C11 Standard", "Written entirely in standard C11 across 22 compilation units totaling 6,320 lines of code."),
            ("Zero External Media Assets", "No images, no .wav files, and no .mp3 files on disk — completely self-contained and architecture-independent."),
            ("Deterministic 60 FPS Budget", "Input polling, network exchange, physics, and graphics drawing must all finish in under 16.6 milliseconds."),
            ("Zero Dynamic Allocation in Game Loop", "All particles, audio buffers, and replay frames use static memory pools to guarantee zero memory fragmentation.")
        ],
        "What This Delivers",
        [
            ("Instant Portability", "Builds cleanly on macOS, Linux (Fedora/Ubuntu), and Windows via a unified Makefile."),
            ("Lightweight Hardware Footprint", "Runs smoothly on low-end hardware with less than 45 MB of RAM and minimal CPU load."),
            ("Rock-Solid Stability", "Memory profiled to ensure zero memory leaks, dangling pointers, or buffer overflows during hours of continuous play."),
            ("Production-Grade Code Quality", "Separates concerns into independent, decoupled C modules with clean public header interfaces.")
        ],
        "Our engine is completely self-contained with no external images or audio files. It runs with zero memory leaks and maintains a strict 16.6-millisecond frame budget to guarantee a smooth 60 FPS experience."
    )

    # ---------------- SLIDE 4: GAMEPLAY MODES ----------------
    add_two_column_slide(
        4,
        "Key Features: Game Modes",
        "Gameplay Variety",
        "Core Play Formats",
        [
            ("Solo Mode", "Play against our 5-state autonomous computer opponent with Easy, Medium, and Hard difficulty presets."),
            ("Local 2-Player", "Two players share the same keyboard on a single computer (Player 1 on W,A,S,D and Player 2 on Arrow Keys)."),
            ("Host LAN Multiplayer", "Turns the computer into an authoritative game server listening on port 50505 over local Wi-Fi or Ethernet."),
            ("Join LAN Multiplayer", "Direct IP-based client connection with automatic handshake sync and 3-second countdown lock.")
        ],
        "Championship & Exhibition Modes",
        [
            ("Tournament Mode (BO3 / BO5)", "Structured championship series in Best-of-3 or Best-of-5 formats with automated match progression."),
            ("Time Attack Challenge", "A 60-second high-score sprint against the clock to score maximum goals under pressure."),
            ("Bot vs. Bot Spectator Demo", "Automated exhibition match where two autonomous computer algorithms compete without human input."),
            ("Interactive Settings Menu", "Configure winning score limits, puck speed, themes, and sound levels persisted to settings.dat.")
        ],
        "The game offers 5 main play formats: single-player solo play, local two-player shared keyboard, cross-laptop LAN play, and multi-game tournaments."
    )

    # ---------------- SLIDE 5: GAMEPLAY POLISH & TELEMETRY ----------------
    add_two_column_slide(
        5,
        "Key Features: Polish & Telemetry",
        "Esports Features & Visual Feel",
        "Interactive Sports Telemetry",
        [
            ("Instant Slow-Mo Replay (V Key)", "Press V during play to immediately review the last 5 seconds of action in smooth 0.35x slow motion."),
            ("Predictive Aim Guide (CTRL Key)", "Hold CTRL to project dotted green lines showing cushion bounce reflections 60 steps into the future."),
            ("Territorial Heatmap (TAB Key)", "Press TAB to toggle a live 24x32 spatial grid showing which player dominated puck possession."),
            ("Live Match Statistics", "Tracks continuous rally volleys, player hit counts, conversion efficiency, and peak recorded puck speed.")
        ],
        "Visual Polish & Game Feel",
        [
            ("Arena Power-Up Pickups", "Floating arena tokens that enlarge paddles, slow down the puck, or temporarily freeze the opponent."),
            ("512-Particle Visual Pool", "Spawns dynamic spark bursts on paddle hits, cushion dust on wall bounces, and goal confetti explosions."),
            ("Four Complete Arena Themes", "Neon (default cyan/red), Retro (arcade amber), Dark (minimalist slate), and Fire (high-contrast orange/gold)."),
            ("Screen Shake & Flash", "Dynamic 14-pixel camera impulse offsets and strobe flashes celebrate every scored goal.")
        ],
        "To make the game feel like a polished sports title, we included instant slow-motion goal replays, an aim-assist bounce guide, live possession heatmaps, and collectible power-ups."
    )

    # ---------------- SLIDE 6: MAIN MODULES OVERVIEW ----------------
    add_two_column_slide(
        6,
        "Main Modules Overview",
        "System Architecture & Modularity",
        "The 5 Core Engine Pillars",
        [
            ("1. Physics Engine (game.c)", "Responsible for paddle acceleration, table boundaries, elastic collisions, and puck kinematics."),
            ("2. LAN Networking (net.c)", "Handles socket creation, non-blocking packet transfer, and Host-Authoritative synchronization."),
            ("3. Solo Opponent Engine (ai.c)", "Controls the autonomous opponent using a 5-state finite state machine with habit adaptation."),
            ("4. Procedural Audio DSP (audio.c)", "Generates raw 44.1 kHz PCM audio waveforms directly in RAM without using audio files."),
            ("5. Rendering & UI (render.c, ui.c)", "Renders 2D OpenGL graphics, UI menus, and coordinates the 512-particle memory pool.")
        ],
        "Decoupled Architectural Design",
        [
            ("Strict Separation of Concerns", "Physics logic never directly touches socket calls; networking never directly executes rendering commands."),
            ("Clean Event Communication", "Subsystems communicate through simple state structs and lightweight event flags (ev_hit, ev_goal)."),
            ("Zero Global Pollution", "Engine state is encapsulated cleanly in GameState, NetState, and MatchStats structs."),
            ("High Maintainability", "Individual modules can be tested, modified, or extended independently without breaking the game loop.")
        ],
        "The entire project is structured into 5 decoupled modules. Each module has one specific job—physics never touches networking, and audio never touches graphics."
    )

    # ---------------- SLIDE 7: MODULE 1 - PHYSICS OVERVIEW ----------------
    add_two_column_slide(
        7,
        "Module 1: Physics Engine (How It Works Overall)",
        "Physics Subsystem (game.c)",
        "Collision & Movement Logic",
        [
            ("Paddle Movement & Boundaries", "Paddles accelerate smoothly up to a maximum speed limit and are strictly clamped to their legal half-courts."),
            ("Circle-to-Circle Collision", "Detects paddle-to-puck contact using clean 2D distance checks between center points and combined radii."),
            ("Elastic Momentum Exchange", "Resolves collisions with an elastic restitution factor of 0.92, ensuring fast and satisfying rebounds."),
            ("Table Boundary Cushions", "Reflects the puck cleanly off side walls while allowing entrance into open goal slots at the table ends.")
        ],
        "Flick Shots & Curving Spin",
        [
            ("Flick Shot Velocity Boost", "Moving the paddle rapidly forward through the puck transfers extra momentum (+0.35x paddle speed) for power shots."),
            ("Curving Spin (Magnus Effect)", "Striking the edge of the puck imparts angular spin, making the puck curve in mid-air just like real air hockey."),
            ("Predictive Bank Trajectory", "Raycasts forward steps into the future, mirroring reflection angles off table walls to assist player aiming."),
            ("Friction & Natural Damping", "Applies subtle air resistance to gradually stabilize wild shots back toward manageable rally speeds.")
        ],
        "Our physics engine handles real-time collisions. When you swing your paddle fast, it imparts a flick bonus for high-speed shots, and hitting the edge causes the puck to curve gracefully in flight."
    )

    # ---------------- SLIDE 8: MODULE 2 - NETWORKING OVERVIEW ----------------
    add_two_column_slide(
        8,
        "Module 2: Multiplayer Networking (How It Works Overall)",
        "Network Subsystem (net.c, protocol.h)",
        "The Host-Authoritative Rule",
        [
            ("The Host Is the Single Boss", "The Host computer acts as the authoritative server, calculating 100% of the physics, collisions, and scoring."),
            ("The Client Is an Input Terminal", "The Client captures player keyboard moves, sends target coordinates, and displays what the host dictates."),
            ("Zero Desynchronization", "Because the client never calculates collisions independently, the puck is never in two different places on the two screens."),
            ("Synchronized Match Start", "Connection handshake triggers a synchronized 3-second countdown lock so both players start at the exact same moment.")
        ],
        "Non-Blocking I/O (Zero Lag Spikes)",
        [
            ("Non-Blocking Socket Calls", "Sockets are set to non-blocking mode — recv() returns immediately if no data is waiting, so the screen never freezes."),
            ("TCP Stream Reassembly", "Custom send and receive loops handle fragmented TCP buffers, ensuring packets are fully transferred without corruption."),
            ("Minimal Network Overhead", "Exchanges lightweight binary packets (12-byte client inputs and 56-byte host snapshots) every 16.6 milliseconds."),
            ("Cross-Platform Sockets", "Supports standard POSIX sockets on Linux/macOS and Winsock2 on Windows for cross-platform LAN matches.")
        ],
        "For multiplayer, the Host acts as the single source of truth. It calculates all physics and broadcasts snapshots to the client. Because sockets are non-blocking, a slow network will never cause the game loop to freeze."
    )

    # ---------------- SLIDE 9: MODULE 3 - SOLO OPPONENT OVERVIEW ----------------
    add_two_column_slide(
        9,
        "Module 3: Solo Opponent / AI (How It Works Overall)",
        "Solo Mode Engine (ai.c)",
        "5 Behavioral States (Finite State Machine)",
        [
            ("1. Defending", "Patrols in a protective curved arc in front of the goal whenever the puck is on the player's side."),
            ("2. Tracking", "Calculates puck trajectory and wall bounces to position itself ahead of incoming shots."),
            ("3. Attacking", "Moves into position behind the puck when it slows down near the center line."),
            ("4. Striking", "Executes a quick recoil windup backwards before lunging forward with an explosive smash toward the player's net."),
            ("5. Resetting", "Smoothly recovers back to its defensive home position after taking a shot.")
        ],
        "Human-Like Intelligence & Learning",
        [
            ("Realistic Reaction Delays", "Features a built-in reaction latency timer so it behaves like a human player rather than an instant-reacting robot."),
            ("Aim Jitter & Imperfections", "Introduces natural positional variations so shots feel dynamic and varied rather than repetitive."),
            ("Spatial Habit Adaptation", "Monitors which corner of the goal you repeatedly score in (left, center, or right) and shifts its stance to protect that area."),
            ("Three Difficulty Presets", "Easy (relaxed pace, wider jitter), Medium (balanced), and Hard (tight prediction and aggressive smashes).")
        ],
        "In Solo Mode, the computer behaves like a real human. It tracks the puck, winds up before striking, and even learns your habits—if you keep scoring on its left corner, it shifts its stance to protect that side."
    )

    # ---------------- SLIDE 10: MODULE 4 - PROCEDURAL AUDIO OVERVIEW ----------------
    add_two_column_slide(
        10,
        "Module 4: Procedural Audio (How It Works Overall)",
        "Audio Subsystem (audio.c)",
        "Zero Media Files on Disk",
        [
            ("No Audio Files Required", "The game folder contains zero .wav or .mp3 files — eliminating missing audio file errors completely."),
            ("Synthesized Directly in Memory", "All sound effects are generated mathematically in RAM at 44.1 kHz, 16-bit PCM quality on startup."),
            ("Waveform Generation", "Combines pure musical sine tones with exponential decay envelopes to produce clean, satisfying tones."),
            ("Mechanical Impact Simulation", "Blends pure tones with short bursts of white noise to recreate the physical clack of plastic hitting plastic.")
        ],
        "Dynamic Volume & Frequency Scaling",
        [
            ("Impulse-Responsive Sound", "Sound volume and pitch dynamically scale based on collision velocity."),
            ("Soft Taps vs. Hard Smashes", "Gentle paddle taps produce soft, low-pitched clicks; high-speed smashes produce loud, sharp cracks."),
            ("Diverse Sound Library", "Procedurally generates paddle hits, wall cushion thuds, countdown beeps, and victory fanfare tones."),
            ("Near-Zero Latency", "Because audio waveforms reside directly in RAM, sounds play with instantaneous hardware response.")
        ],
        "Instead of loading sound files from the hard drive, we synthesize our sound effects directly in RAM using code. Fast paddle hits automatically trigger louder, higher-pitched impact sounds."
    )

    # ---------------- SLIDE 11: MODULE 5 - VISUALS & ANALYTICS OVERVIEW ----------------
    add_two_column_slide(
        11,
        "Module 5: Visuals & Analytics (How It Works Overall)",
        "Graphics & Telemetry (render.c, stats.c, replay.c)",
        "Visual Polish & Particle Pool",
        [
            ("512-Particle Static Memory Pool", "Fixed array pool for sparks, cushion dust, and goal confetti — zero memory allocation during gameplay."),
            ("Goal Strobe & Screen Shake", "Scoring triggers dynamic camera shake and flash illumination to create high gameplay impact."),
            ("Translucent Vector UI", "Glassmorphic control panels, glowing titles, and animated menu selectors rendered via hardware OpenGL."),
            ("Instant Slow-Mo Memory Buffer", "A 300-frame circular ring buffer continuously records the last 5 seconds in RAM for instant V-key replays.")
        ],
        "Data Analytics & Persistence",
        [
            ("24x32 Territorial Heatmap", "Tracks puck occupancy across a 2D spatial grid with exponential decay to visualize territory control."),
            ("Live Match Telemetry", "Monitors total rallies, hits, conversion efficiency, and peak recorded puck speed in real time."),
            ("Lifetime Disk Records (airhockey.dat)", "Saves all-time records (fastest shot ever, longest rally ever, matches played) permanently to disk."),
            ("User Configuration (settings.dat)", "Persists winning score limits, speeds, and theme selections across game restarts.")
        ],
        "Our graphics engine uses pre-allocated particle pools for goal confetti and impact sparks. It also tracks live match statistics like top shot speed and longest rally, saving your all-time records to disk."
    )

    # ---------------- SLIDE 12: LIVE DEMONSTRATION ----------------
    slide12 = prs.slides.add_slide(blank_layout)
    add_header(slide12, "Live Game Demonstration", "Interactive System Verification")
    add_footer(slide12, 12)

    card_d = slide12.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(0.8), Inches(1.8), Inches(11.7), Inches(4.8))
    card_d.fill.solid()
    card_d.fill.fore_color.rgb = C_BG_CARD
    card_d.line.color.rgb = C_SKY
    card_d.line.width = Pt(2.0)
    tf_d = card_d.text_frame
    tf_d.word_wrap = True

    p_dt = tf_d.paragraphs[0]
    p_dt.text = "WHAT WE WILL DEMONSTRATE LIVE ON SCREEN"
    p_dt.font.size = Pt(18)
    p_dt.font.bold = True
    p_dt.font.color.rgb = C_BLUE

    demo_steps = [
        ("1. Solo Mode Gameplay", "Demonstrate paddle responsiveness, AI defensive tracking, strike windup, and habit adaptation."),
        ("2. Flick Shots & Curving Spin", "Execute high-speed power smashes (+0.35x boost) and demonstrate curving Magnus effect ball trajectory."),
        ("3. Predictive Trajectory Guide", "Hold LEFT CTRL to display real-time cushion bank reflections and angle-of-reflection raycasting."),
        ("4. Instant Slow-Mo Replay", "Score a goal and press 'V' to replay the last 5 seconds in smooth 0.35x slow motion from the RAM buffer."),
        ("5. Territorial Heatmap Overlay", "Press TAB during play to reveal the 24x32 court occupancy density grid showing territorial dominance."),
        ("6. Cross-Laptop LAN Multiplayer", "Connect two laptops (Fedora Linux vs. Windows) over local Wi-Fi to prove zero-desync synchronization.")
    ]

    for d_title, d_desc in demo_steps:
        p = tf_d.add_paragraph()
        p.space_before = Pt(12)
        r1 = p.add_run()
        r1.text = "✔  " + d_title + " — "
        r1.font.bold = True
        r1.font.size = Pt(13)
        r1.font.color.rgb = C_TEXT_DARK
        r2 = p.add_run()
        r2.text = d_desc
        r2.font.size = Pt(13)
        r2.font.color.rgb = C_TEXT_DARK

    slide12.notes_slide.notes_text_frame.text = (
        "We will now switch over to our live engine demonstration to show our physics sub-stepping, Solo Mode opponent, "
        "instant replay buffer, and LAN multiplayer in action."
    )

    # ---------------- SLIDE 13: PROBLEM 1 - TUNNELING ----------------
    add_two_column_slide(
        13,
        "Problem 1: Puck Passing Through Walls (Tunneling)",
        "Challenges Faced & Solutions",
        "The Problem We Encountered",
        [
            ("The High-Velocity Issue", "At top speed (over 1200 px/s), a fast puck moves more than 20 pixels in a single 16.6ms video frame."),
            ("The Tunneling Bug", "Because walls are thin (10px) and paddles are small, the puck would jump completely across them in one frame."),
            ("Game-Breaking Consequence", "Pucks were flying completely outside the rink or passing straight through paddles without registering hits."),
            ("Why Standard Checks Failed", "Traditional single-step collision checks only look at where the puck is right now, not where it was along the way.")
        ],
        "How We Solved It (Dynamic Sub-Stepping)",
        [
            ("Continuous Collision Detection (CCD)", "Instead of moving the puck once per frame, we divide the frame into smaller micro-steps."),
            ("Dynamic Time Slicing", "The engine checks the puck's current speed and dynamically slices the frame into 1 to 6 smaller steps."),
            ("Small Displacements", "In each micro-step, the puck moves only 3 to 4 pixels — smaller than the thickness of any boundary."),
            ("Result", "Guarantees 100% collision containment without tunneling, even during maximum-speed power shots.")
        ],
        "Our first major problem was collision tunneling. At top speed, the puck would jump straight through walls. We solved this by dividing each frame into micro-steps, so the puck moves just a few pixels at a time."
    )

    # ---------------- SLIDE 14: PROBLEM 2 - DESYNC & STALLS ----------------
    add_two_column_slide(
        14,
        "Problem 2: Multiplayer Desync & Screen Freezing",
        "Challenges Faced & Solutions",
        "The Problem We Encountered",
        [
            ("Split-Screen Divergence", "Running independent physics calculations on two computers caused slight floating-point differences over time."),
            ("The Desync Result", "After a few rallies, Player 1 saw the puck in the top corner while Player 2 saw it in the middle."),
            ("Socket Freezing Spikes", "Standard network socket calls pause program execution until bytes arrive from the network."),
            ("The Stalling Result", "Any minor Wi-Fi jitter caused the frame rate to drop to zero, freezing the screen completely.")
        ],
        "How We Solved It (Authority & Non-Blocking I/O)",
        [
            ("Enforced Host Authority", "The Host computer is the single physical authority. The client never calculates collisions independently."),
            ("Non-Blocking POSIX Sockets", "Configured sockets with O_NONBLOCK — if data isn't ready, the game loop never waits and keeps running at 60 FPS."),
            ("Stream Buffer Loops", "Built send_all() and recv_all() loops that handle partial TCP chunks, preventing packet corruption."),
            ("Result", "Zero desynchronization between screens and smooth 60 FPS gameplay even over standard wireless connections.")
        ],
        "Our second challenge was multiplayer desync and screen freezing. We solved this by making the host the single physical authority and setting our sockets to non-blocking mode so the game loop never waits or stalls."
    )

    # ---------------- SLIDE 15: PROBLEM 3 - AUDIO LATENCY & ASSETS ----------------
    add_two_column_slide(
        15,
        "Problem 3: Missing Audio Files & Sound Latency",
        "Challenges Faced & Solutions",
        "The Problem We Encountered",
        [
            ("Path Incompatibility", "Loading external .wav or .mp3 audio files caused path errors when moving between macOS, Linux, and Windows."),
            ("Missing File Crashes", "If an audio file was missing or renamed, the game failed to launch or crashed unexpectedly."),
            ("Noticeable Audio Latency", "Loading and reading audio buffers from the hard drive caused noticeable delay during rapid wall volleys."),
            ("Deployment Overhead", "Packaging dozens of asset files made deploying and running the project fragile across different operating systems.")
        ],
        "How We Solved It (In-Memory Sound Synthesis)",
        [
            ("Zero Asset Dependency", "Completely eliminated all external audio file requirements from the project."),
            ("Procedural RAM Synthesis", "Wrote a mathematical synthesizer in audio.c that generates 16-bit PCM waveform buffers directly in RAM on startup."),
            ("Instantaneous Response", "Because sound data lives directly in computer memory, playback latency is near zero on collisions."),
            ("Result", "The entire game builds into a single self-contained executable that runs anywhere without asset path issues.")
        ],
        "Our third challenge was audio file path errors and sound delay. We solved this by generating our audio directly in computer memory using code, completely removing the need for external sound files."
    )

    # ---------------- SLIDE 16: CONCLUSION & TAKEAWAYS ----------------
    slide16 = prs.slides.add_slide(blank_layout)
    bg16 = slide16.shapes.add_shape(MSO_SHAPE.RECTANGLE, 0, 0, Inches(13.333), Inches(7.5))
    bg16.fill.solid()
    bg16.fill.fore_color.rgb = C_NAVY
    bg16.line.color.rgb = C_NAVY

    # Decorative Border
    d_rect16 = slide16.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(0.8), Inches(0.8), Inches(11.733), Inches(5.9))
    d_rect16.fill.background()
    d_rect16.line.color.rgb = RGBColor(51, 65, 85)
    d_rect16.line.width = Pt(1.5)

    tb16 = slide16.shapes.add_textbox(Inches(1.2), Inches(1.2), Inches(11.0), Inches(5.2))
    tf16 = tb16.text_frame
    tf16.word_wrap = True

    p_e0 = tf16.paragraphs[0]
    p_e0.text = "PROJECT SUMMARY & CONCLUSION"
    p_e0.font.size = Pt(28)
    p_e0.font.bold = True
    p_e0.font.color.rgb = C_CYAN

    achievements = [
        "Proved that high-performance, real-time multiplayer systems can be built in pure C11 without external game engines.",
        "Eliminated network desynchronization using a robust Host-Authoritative model and non-blocking sockets.",
        "Solved high-velocity collision tunneling bugs using dynamic continuous sub-stepping.",
        "Synthesized 100% of game audio mathematically in RAM, eliminating all external media asset files.",
        "Created an intelligent, human-like Solo Mode opponent with strike windup and adaptive habit tracking.",
        "Delivered a cross-platform, zero-memory-leak simulation engine running at a rock-solid 60 FPS on any computer."
    ]

    for a in achievements:
        p = tf16.add_paragraph()
        p.space_before = Pt(12)
        r = p.add_run()
        r.text = "✔   " + a
        r.font.size = Pt(14)
        r.font.color.rgb = C_BG_WHITE

    p_thanks = tf16.add_paragraph()
    p_thanks.space_before = Pt(28)
    r_th = p_thanks.add_run()
    r_th.text = "THANK YOU!   •   QUESTIONS & ANSWERS"
    r_th.font.size = Pt(22)
    r_th.font.bold = True
    r_th.font.color.rgb = RGBColor(254, 240, 138)

    slide16.notes_slide.notes_text_frame.text = (
        "To conclude, this project allowed our team to master real-world systems programming, networking, physics, "
        "and signal processing in pure C. Thank you for your time, and we now welcome any questions."
    )

    prs.save(PPT_PATH)
    print(f"Successfully generated clean 16-slide presentation at: {PPT_PATH}")

if __name__ == "__main__":
    create_presentation()
