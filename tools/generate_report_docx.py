import os
import sys
from docx import Document
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT
from docx.oxml import OxmlElement, parse_xml
from docx.oxml.ns import nsdecls, qn

def set_cell_background(cell, fill_hex):
    shading_elm = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{fill_hex}"/>')
    cell._tc.get_or_add_tcPr().append(shading_elm)

def set_cell_margins(cell, top=100, bottom=100, left=140, right=140):
    tcPr = cell._tc.get_or_add_tcPr()
    tcMar = OxmlElement('w:tcMar')
    for m, val in [('top', top), ('bottom', bottom), ('left', left), ('right', right)]:
        node = OxmlElement(f'w:{m}')
        node.set(qn('w:w'), str(val))
        node.set(qn('w:type'), 'dxa')
        tcMar.append(node)
    tcPr.append(tcMar)

def set_cell_border(cell, **kwargs):
    tcPr = cell._tc.get_or_add_tcPr()
    tcBorders = OxmlElement('w:tcBorders')
    for b_name in ['top', 'left', 'bottom', 'right']:
        if b_name in kwargs:
            edge = OxmlElement(f'w:{b_name}')
            for key, val in kwargs[b_name].items():
                edge.set(qn(f'w:{key}'), str(val))
            tcBorders.append(edge)
        else:
            edge = OxmlElement(f'w:{b_name}')
            edge.set(qn('w:val'), 'none')
            tcBorders.append(edge)
    tcPr.append(tcBorders)

def add_heading_styled(doc, text, level):
    h = doc.add_heading(text, level=level)
    h.paragraph_format.keep_with_next = True
    h.paragraph_format.space_before = Pt(14)
    h.paragraph_format.space_after = Pt(6)
    run = h.runs[0]
    if level == 1:
        run.font.size = Pt(17)
        run.font.color.rgb = RGBColor(24, 43, 73)
        run.bold = True
    elif level == 2:
        run.font.size = Pt(13)
        run.font.color.rgb = RGBColor(41, 70, 115)
        run.bold = True
    elif level == 3:
        run.font.size = Pt(11)
        run.font.color.rgb = RGBColor(50, 50, 50)
        run.bold = True
    return h

def format_table(table, col_widths, header_bg="1E3A8A"):
    table.alignment = WD_TABLE_ALIGNMENT.CENTER
    header_tr = table.rows[0]
    header_tr._tr.get_or_add_trPr().append(parse_xml(f'<w:tblHeader {nsdecls("w")}/>'))
    for i, cell in enumerate(header_tr.cells):
        set_cell_background(cell, header_bg)
        set_cell_margins(cell, 120, 120, 140, 140)
        cell.width = col_widths[i]
        for p in cell.paragraphs:
            p.paragraph_format.space_before = Pt(2)
            p.paragraph_format.space_after = Pt(2)
            for run in p.runs:
                run.font.bold = True
                run.font.color.rgb = RGBColor(255, 255, 255)
                run.font.size = Pt(9.5)
    
    for r_idx, row in enumerate(table.rows[1:], start=1):
        bg = "F8FAFC" if r_idx % 2 == 1 else "FFFFFF"
        for i, cell in enumerate(row.cells):
            set_cell_background(cell, bg)
            set_cell_margins(cell, 90, 90, 130, 130)
            cell.width = col_widths[i]
            for p in cell.paragraphs:
                p.paragraph_format.space_before = Pt(2)
                p.paragraph_format.space_after = Pt(2)
                for run in p.runs:
                    run.font.size = Pt(9.0)

def add_diagram_figure(doc, img_path, caption, desc, width_inches=5.8):
    if os.path.exists(img_path):
        p_img = doc.add_paragraph()
        p_img.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p_img.paragraph_format.space_before = Pt(8)
        p_img.paragraph_format.space_after = Pt(4)
        run = p_img.add_run()
        run.add_picture(img_path, width=Inches(width_inches))
        
        p_cap = doc.add_paragraph()
        p_cap.alignment = WD_ALIGN_PARAGRAPH.CENTER
        p_cap.paragraph_format.space_after = Pt(3)
        c_run = p_cap.add_run(caption)
        c_run.font.bold = True
        c_run.font.size = Pt(9.5)
        c_run.font.color.rgb = RGBColor(24, 43, 73)
        
        p_desc = doc.add_paragraph()
        p_desc.paragraph_format.space_after = Pt(12)
        d_run = p_desc.add_run(desc)
        d_run.font.size = Pt(9.0)
        d_run.font.italic = True
        d_run.font.color.rgb = RGBColor(70, 70, 70)

def build_report():
    doc = Document()
    
    for s in doc.sections:
        s.top_margin = Inches(1.0)
        s.bottom_margin = Inches(1.0)
        s.left_margin = Inches(1.0)
        s.right_margin = Inches(1.0)
        
    normal_style = doc.styles['Normal']
    normal_style.font.name = 'Calibri'
    normal_style.font.size = Pt(10.5)
    normal_style.font.color.rgb = RGBColor(40, 40, 40)
    normal_style.paragraph_format.line_spacing = 1.15
    normal_style.paragraph_format.space_after = Pt(5)

    # ==================== COVER PAGE ====================
    # 1. Institution Header
    p_inst = doc.add_paragraph()
    p_inst.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_inst.paragraph_format.space_before = Pt(8)
    p_inst.paragraph_format.space_after = Pt(2)
    r_inst = p_inst.add_run("[ COLLEGE / UNIVERSITY NAME HERE ]")
    r_inst.font.name = "Calibri"
    r_inst.font.size = Pt(16)
    r_inst.font.bold = True
    r_inst.font.color.rgb = RGBColor(30, 58, 138)

    p_subinst = doc.add_paragraph()
    p_subinst.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_subinst.paragraph_format.space_after = Pt(2)
    r_sub = p_subinst.add_run("Approved by AICTE, New Delhi & Affiliated to [University Name]\nDEPARTMENT OF COMPUTER APPLICATIONS")
    r_sub.font.name = "Calibri"
    r_sub.font.size = Pt(10.5)
    r_sub.font.bold = True
    r_sub.font.color.rgb = RGBColor(71, 85, 105)

    p_session = doc.add_paragraph()
    p_session.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_session.paragraph_format.space_after = Pt(10)
    r_ses = p_session.add_run("ACADEMIC YEAR 2026 – 2027")
    r_ses.font.name = "Calibri"
    r_ses.font.size = Pt(9.5)
    r_ses.font.bold = True
    r_ses.font.color.rgb = RGBColor(148, 163, 184)

    # Accent divider line
    t_rule = doc.add_table(rows=1, cols=1)
    t_rule.alignment = WD_TABLE_ALIGNMENT.CENTER
    cell_rule = t_rule.rows[0].cells[0]
    cell_rule.width = Inches(6.5)
    set_cell_background(cell_rule, "1E3A8A")
    set_cell_margins(cell_rule, top=20, bottom=20, left=0, right=0)
    p_empty = cell_rule.paragraphs[0]
    p_empty.paragraph_format.space_before = Pt(0)
    p_empty.paragraph_format.space_after = Pt(0)
    r_empty = p_empty.add_run()
    r_empty.font.size = Pt(2)

    # 2. Title Card Block
    p_sp = doc.add_paragraph()
    p_sp.paragraph_format.space_before = Pt(6)
    p_sp.paragraph_format.space_after = Pt(0)

    t_card = doc.add_table(rows=1, cols=1)
    t_card.alignment = WD_TABLE_ALIGNMENT.CENTER
    c_card = t_card.rows[0].cells[0]
    c_card.width = Inches(6.5)
    set_cell_background(c_card, "F8FAFC")
    set_cell_margins(c_card, top=140, bottom=140, left=180, right=180)
    set_cell_border(c_card, 
                    top={'sz': 8, 'val': 'single', 'color': 'CBD5E1'},
                    bottom={'sz': 8, 'val': 'single', 'color': 'CBD5E1'},
                    left={'sz': 18, 'val': 'single', 'color': '1E3A8A'},
                    right={'sz': 8, 'val': 'single', 'color': 'CBD5E1'})

    p_type = c_card.paragraphs[0]
    p_type.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_type.paragraph_format.space_before = Pt(0)
    p_type.paragraph_format.space_after = Pt(3)
    r_type = p_type.add_run("ACADEMIC PROJECT REPORT")
    r_type.font.name = "Calibri"
    r_type.font.size = Pt(10)
    r_type.font.bold = True
    r_type.font.color.rgb = RGBColor(37, 99, 235)

    p_t = c_card.add_paragraph()
    p_t.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_t.paragraph_format.space_before = Pt(2)
    p_t.paragraph_format.space_after = Pt(3)
    r_t = p_t.add_run("AIR HOCKEY")
    r_t.font.name = "Calibri"
    r_t.font.size = Pt(24)
    r_t.font.bold = True
    r_t.font.color.rgb = RGBColor(15, 23, 42)

    p_sub = c_card.add_paragraph()
    p_sub.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_sub.paragraph_format.space_before = Pt(0)
    p_sub.paragraph_format.space_after = Pt(6)
    r_sub = p_sub.add_run("A Real-Time 2D LAN Multiplayer & Adaptive Simulation Engine")
    r_sub.font.name = "Calibri"
    r_sub.font.size = Pt(11.5)
    r_sub.font.bold = True
    r_sub.font.color.rgb = RGBColor(30, 58, 138)

    p_tech = c_card.add_paragraph()
    p_tech.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_tech.paragraph_format.space_before = Pt(0)
    p_tech.paragraph_format.space_after = Pt(0)
    r_tech = p_tech.add_run("Pure C11  •  OpenGL / Raylib  •  Raw TCP Sockets  •  Procedural DSP Audio")
    r_tech.font.name = "Calibri"
    r_tech.font.size = Pt(9.0)
    r_tech.font.italic = True
    r_tech.font.color.rgb = RGBColor(100, 116, 139)

    # 3. Submission Statement
    p_deg = doc.add_paragraph()
    p_deg.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_deg.paragraph_format.space_before = Pt(10)
    p_deg.paragraph_format.space_after = Pt(2)
    r_deg1 = p_deg.add_run("Submitted in partial fulfillment of the requirements for the award of the degree of\n")
    r_deg1.font.name = "Calibri"
    r_deg1.font.size = Pt(10)
    r_deg1.font.italic = True
    r_deg1.font.color.rgb = RGBColor(51, 65, 85)

    r_deg2 = p_deg.add_run("MASTER OF COMPUTER APPLICATIONS (MCA)")
    r_deg2.font.name = "Calibri"
    r_deg2.font.size = Pt(12.5)
    r_deg2.font.bold = True
    r_deg2.font.color.rgb = RGBColor(30, 58, 138)

    # 4. Members Table
    m_p = doc.add_paragraph()
    m_p.paragraph_format.space_before = Pt(8)
    m_p.paragraph_format.space_after = Pt(4)
    m_run = m_p.add_run("PROJECT TEAM & CONTRIBUTION MATRIX:")
    m_run.font.bold = True
    m_run.font.size = Pt(10)
    m_run.font.color.rgb = RGBColor(30, 58, 138)

    table = doc.add_table(rows=5, cols=4)
    col_widths = [Inches(1.15), Inches(1.75), Inches(1.8), Inches(1.8)]
    headers = ["Roll No.", "Student Name", "Engineering Role", "Assigned Modules"]
    for i, h in enumerate(headers):
        table.rows[0].cells[i].text = h

    data = [
        ["MCA-2026-01", "Ravi Chauhan (Lead)", "Network & Systems Architect", "main.c, net.c, protocol.h"],
        ["MCA-2026-02", "Team Member 2", "Physics & Simulation Engineer", "game.c, trajectory.c, input.c"],
        ["MCA-2026-03", "Team Member 3", "Graphics & Audio DSP Engineer", "render.c, audio.c, dynaudio.c"],
        ["MCA-2026-04", "Team Member 4", "Solo Mode & Data Systems Engineer", "solomode.c, replay.c, stats.c"]
    ]
    for r_idx, row_data in enumerate(data, start=1):
        for c_idx, val in enumerate(row_data):
            table.rows[r_idx].cells[c_idx].text = val
    format_table(table, col_widths, "1E3A8A")

    # 5. Dual Guidance & Endorsement Block
    p_sp2 = doc.add_paragraph()
    p_sp2.paragraph_format.space_before = Pt(10)
    p_sp2.paragraph_format.space_after = Pt(0)

    t_guide = doc.add_table(rows=1, cols=2)
    t_guide.alignment = WD_TABLE_ALIGNMENT.CENTER
    c_left = t_guide.rows[0].cells[0]
    c_right = t_guide.rows[0].cells[1]
    c_left.width = Inches(3.25)
    c_right.width = Inches(3.25)
    set_cell_background(c_left, "F1F5F9")
    set_cell_background(c_right, "F1F5F9")
    set_cell_margins(c_left, top=80, bottom=80, left=120, right=120)
    set_cell_margins(c_right, top=80, bottom=80, left=120, right=120)
    set_cell_border(c_left, 
                    top={'sz': 6, 'val': 'single', 'color': 'CBD5E1'},
                    bottom={'sz': 6, 'val': 'single', 'color': 'CBD5E1'},
                    left={'sz': 12, 'val': 'single', 'color': '1E3A8A'},
                    right={'sz': 4, 'val': 'single', 'color': 'CBD5E1'})
    set_cell_border(c_right, 
                    top={'sz': 6, 'val': 'single', 'color': 'CBD5E1'},
                    bottom={'sz': 6, 'val': 'single', 'color': 'CBD5E1'},
                    left={'sz': 4, 'val': 'single', 'color': 'CBD5E1'},
                    right={'sz': 12, 'val': 'single', 'color': '1E3A8A'})

    p_gl = c_left.paragraphs[0]
    p_gl.paragraph_format.space_before = Pt(0)
    p_gl.paragraph_format.space_after = Pt(2)
    r1 = p_gl.add_run("Under the Guidance of:\n")
    r1.font.name = "Calibri"
    r1.font.size = Pt(9.0)
    r1.font.italic = True
    r1.font.color.rgb = RGBColor(100, 116, 139)
    r2 = p_gl.add_run("[Faculty Guide Name]\n")
    r2.font.name = "Calibri"
    r2.font.size = Pt(10.5)
    r2.font.bold = True
    r2.font.color.rgb = RGBColor(15, 23, 42)
    r3 = p_gl.add_run("Assistant Professor\nDept. of Computer Applications")
    r3.font.name = "Calibri"
    r3.font.size = Pt(8.0)
    r3.font.color.rgb = RGBColor(71, 85, 105)

    p_gr = c_right.paragraphs[0]
    p_gr.paragraph_format.space_before = Pt(0)
    p_gr.paragraph_format.space_after = Pt(2)
    r4 = p_gr.add_run("Approved & Endorsed by:\n")
    r4.font.name = "Calibri"
    r4.font.size = Pt(9.0)
    r4.font.italic = True
    r4.font.color.rgb = RGBColor(100, 116, 139)
    r5 = p_gr.add_run("[Head of Department / HOD]\n")
    r5.font.name = "Calibri"
    r5.font.size = Pt(10.5)
    r5.font.bold = True
    r5.font.color.rgb = RGBColor(15, 23, 42)
    r6 = p_gr.add_run("Professor & Head\nDept. of Computer Applications")
    r6.font.name = "Calibri"
    r6.font.size = Pt(8.0)
    r6.font.color.rgb = RGBColor(71, 85, 105)

    p_foot = doc.add_paragraph()
    p_foot.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_foot.paragraph_format.space_before = Pt(10)
    p_foot.paragraph_format.space_after = Pt(0)
    r_foot = p_foot.add_run("DEPARTMENT OF COMPUTER APPLICATIONS  •  NOVEMBER 2026\n[City, State, India]")
    r_foot.font.name = "Calibri"
    r_foot.font.size = Pt(8.5)
    r_foot.font.bold = True
    r_foot.font.color.rgb = RGBColor(148, 163, 184)

    doc.add_page_break()

    # ==================== CHAPTER 1 ====================
    add_heading_styled(doc, "Chapter 1: Motivation Behind the Project", level=1)
    doc.add_paragraph(
        "The motivation behind developing this Air Hockey simulation engine stems from a strong desire to explore "
        "low-level systems programming, real-time physics, and network engineering from the ground up using pure C11."
    )
    doc.add_paragraph("The core motivations guiding this project are:")
    
    bullet_items = [
        ("Mastering Low-Level Systems and Memory Control: ", "Gaining deep, practical expertise in manual memory management, pointer arithmetic, and direct hardware execution without relying on automated garbage collectors or heavy runtime abstractions."),
        ("Building a Real-Time Multiplayer Network Engine: ", "Understanding and implementing real-time client-server communication using raw TCP sockets, focusing on non-blocking network I/O, custom packet serialization, and synchronized gameplay across multiple machines with zero lag."),
        ("Implementing Physics and Mathematics from First Principles: ", "Understanding how game mechanics function under the hood by implementing 2D vector mathematics, elastic collision resolution, and sub-stepped physics integration directly using mathematical formulas rather than a pre-built physics engine."),
        ("Creating a Lightweight, Self-Contained Application: ", "Engineering an ultra-lightweight, efficient application (under 10 MB) that requires zero external asset files—generating all visual effects, particle systems, and audio waveforms procedurally in code through digital signal processing (DSP)."),
        ("Developing Seamless Cross-Platform Portability: ", "Writing clean, standard C code that compiles natively on both Linux (POSIX) and Windows (Winsock2), allowing different operating systems to connect and play together seamlessly over a local area network.")
    ]
    for b_title, b_desc in bullet_items:
        bp = doc.add_paragraph(style='List Bullet')
        r1 = bp.add_run(b_title)
        r1.bold = True
        r1.font.color.rgb = RGBColor(24, 43, 73)
        bp.add_run(b_desc)

    doc.add_paragraph(
        "By focusing on these core engineering motivations, this project delivers a robust, self-contained, and performant "
        "real-time sports simulation engine that unifies mathematical modeling, network synchronization, and systems programming into a single cohesive system."
    )

    # ==================== CHAPTER 2 ====================
    add_heading_styled(doc, "Chapter 2: Project Objectives", level=1)
    doc.add_paragraph("The primary objectives of this project include:")
    objs = [
        ("Sub-Stepped 2D Physics Engine: ", "Implement continuous collision detection (CCD) with up to 6 micro-steps per frame to prevent high-speed tunneling, along with momentum transfer ('flick shots') and Magnus effect curve spin physics."),
        ("Host-Authoritative LAN Multiplayer: ", "Design a zero-desync client-server architecture over cross-platform TCP sockets (POSIX on Linux/macOS and Winsock2 on Windows, port 50505) with non-blocking I/O ensuring the 60 FPS game loop never stalls."),
        ("Solo Mode Opponent Mechanics: ", "Develop an autonomous state machine (DEFEND, INTERCEPT, ATTACK, STRIKE) in Solo Mode that dynamically learns player scoring tendencies, featuring predictive cushion raycasting and spatial habit adaptation."),
        ("100% Procedural Audio DSP: ", "Synthesize all sound effects directly in RAM using raw 44.1 kHz PCM waveforms (sine waves, white noise, and exponential decay envelopes), removing all external audio file dependencies."),
        ("Esports Telemetry & Polish: ", "Provide high visual juice with a static 512-particle pool, live 24x32 court heatmaps, instant slow-motion replay circular ring buffers, and 4 arena themes.")
    ]
    for o_title, o_desc in objs:
        op = doc.add_paragraph(style='List Bullet')
        r1 = op.add_run(o_title)
        r1.bold = True
        r1.font.color.rgb = RGBColor(24, 43, 73)
        op.add_run(o_desc)

    doc.add_page_break()

    # ==================== CHAPTER 3 ====================
    add_heading_styled(doc, "Chapter 3: System Architecture & Flowcharts", level=1)
    doc.add_paragraph(
        "The project architecture is structured around a decoupled layer design. Every critical responsibility—from network packet serialization "
        "to physical sub-stepping and waveform synthesis—is isolated in specialized modules to maintain stability, zero memory leakage, and deterministic 60 FPS performance."
    )

    add_heading_styled(doc, "3.1 High-Level Architecture Overview", level=2)
    doc.add_paragraph(
        "The software architecture establishes a clear separation between the Host (which acts as server and authoritative physics simulator) and the Client (which acts as a thin input/output terminal). "
        "The Host runs the core game loop, accepts client paddle targets, computes all physics and collisions, and replicates the resulting world snapshot to the client:"
    )

    add_diagram_figure(
        doc,
        "/Users/ravichauhan/Desktop/COLLEGE/C/PROJ/diagrams/01_high_level_arch.png",
        "Figure 3.1: High-Level System Architecture (Mermaid Flowchart)",
        "Illustrates the interaction between Client Input, Network Sockets, Host Physics Engine, Solo Mode Opponent Controller, Procedural Audio DSP, and Raylib Rendering.",
        width_inches=5.8
    )

    doc.add_page_break()

    add_heading_styled(doc, "3.2 Multiplayer Synchronization Sequence & Protocol", level=2)
    doc.add_paragraph(
        "To prevent desynchronization in real-time LAN play, the network engine employs a Host-Authoritative synchronization protocol over non-blocking cross-platform TCP sockets on port 50505. "
        "The connection and communication cycle proceeds as follows:"
    )
    
    add_diagram_figure(
        doc,
        "/Users/ravichauhan/Desktop/COLLEGE/C/PROJ/diagrams/02_sync_flow.png",
        "Figure 3.2: Network Communication & Frame Synchronization (Mermaid Sequence Diagram)",
        "Details the handshake, 3-second countdown lock, client input transmission (MSG_INPUT), and host authoritative snapshot broadcast (MSG_STATE).",
        width_inches=5.6
    )

    doc.add_paragraph(
        "Key aspects of the synchronization protocol:\n"
        "1. Handshake Phase: When the client connects, the host accepts the connection via accept(), sets the socket to O_NONBLOCK, and transmits a MSG_START packet. Both machines start a 3-second countdown timer during which puck physics are stationary.\n"
        "2. Client Transmission: On every 16.6 ms frame, the client reads its local keyboard/mouse input and transmits a 12-byte MSG_INPUT packet containing (client_tx, client_ty).\n"
        "3. Host Authoritative Step: The host drains all incoming client packets using net_poll_input(), applies the client target to Player 2's paddle, runs the physics engine across 6 sub-steps, and transmits a 56-byte MSG_STATE snapshot.\n"
        "4. Non-Blocking Resilience: Sockets use fcntl(fd, F_SETFL, O_NONBLOCK). If a packet is slightly delayed, recv_all() returns immediately with EAGAIN/EWOULDBLOCK instead of freezing the frame loop."
    )

    doc.add_page_break()

    add_heading_styled(doc, "3.3 Continuous Collision Detection (CCD) & Sub-Stepping Pipeline", level=2)
    doc.add_paragraph(
        "Standard Euler integration samples physics once per video frame (dt ≈ 0.016s). At maximum puck speed (1400 pixels/second), the puck travels over 23 pixels in a single frame, "
        "easily passing entirely through thin paddles (radius 28px) or table walls (thickness 10px). This project implements dynamic sub-stepping:"
    )

    add_diagram_figure(
        doc,
        "/Users/ravichauhan/Desktop/COLLEGE/C/PROJ/diagrams/03a_physics_substep.png",
        "Figure 3.3a: Dynamic Sub-Step Time Slicing & Loop Pipeline (Mermaid Flowchart)",
        "Illustrates the dynamic micro-step calculation, time slicing (sdt = dt / steps), and the multi-iteration sub-stepping loop.",
        width_inches=5.8
    )

    add_diagram_figure(
        doc,
        "/Users/ravichauhan/Desktop/COLLEGE/C/PROJ/diagrams/03b_collision_magnus.png",
        "Figure 3.3b: Contact Impulse Resolution, Boundary Cushions & Magnus Flight (Mermaid Flowchart)",
        "Details normal impulse resolution, flick shot boosts, tangential friction spin transfer, goal slots, and Magnus curving flight.",
        width_inches=5.8
    )

    doc.add_paragraph(
        "The number of sub-steps is dynamically calculated each frame using:\n"
        "    steps = 1 + (int)(speed * dt / (PUCK_R * 0.5f))\n"
        "clamped between 1 and 6 steps. By advancing the simulation in micro-intervals (sdt = dt / steps), the puck's displacement per step never exceeds half its radius, "
        "guaranteeing 100% collision containment without tunneling."
    )

    doc.add_page_break()

    add_heading_styled(doc, "3.4 Solo Mode: Autonomous Finite State Machine & Adaptive Learning", level=2)
    doc.add_paragraph(
        "In Solo Mode (Single Player vs. Computer), the autonomous opponent paddle is governed by an intelligent Finite State Machine (FSM) that models realistic human player behavior. "
        "Unlike basic pong bots that directly lock their position to the puck, our Solo opponent exhibits reaction latency, tracking jitter, strike windup, and habit adaptation:"
    )

    add_diagram_figure(
        doc,
        "/Users/ravichauhan/Desktop/COLLEGE/C/PROJ/diagrams/04_ai_state_machine.png",
        "Figure 3.4: Autonomous State Machine & Habit Adaptation (Mermaid State Diagram)",
        "Demonstrates transitions between DEFEND, INTERCEPT, ATTACK, STRIKE (with windup), and RESET states based on court positioning and puck velocity.",
        width_inches=5.8
    )

    doc.add_paragraph(
        "Adaptive Habit Tracking: The Solo opponent controller records where the player scores (left, center, or right third of the goal) and where the player prefers to take shots from. "
        "Over time, the opponent shifts its resting defensive arc toward the player's favored scoring zone, actively forcing the player to adapt their strategy."
    )

    doc.add_page_break()

    # ==================== CHAPTER 4 ====================
    add_heading_styled(doc, "Chapter 4: Detailed Subsystem Breakdown", level=1)
    doc.add_paragraph(
        "The project consists of 22 pairs of C source (.c) and header (.h) files totaling ~6,320 lines of code. "
        "Below is the complete engineering breakdown of each core subsystem:"
    )

    modules_deep = [
        ("4.1 Master Orchestrator (src/main.c, src/features.h)",
         "src/main.c acts as the director of the entire game engine. It initializes the 600x920 raylib window, sets up audio and particle subsystems, "
         "loads saved disk preferences, and drives the 60 FPS game loop. It implements a finite screen state manager governing 12 screens (SCREEN_MENU, "
         "SCREEN_PLAYING, SCREEN_HOST_WAIT, SCREEN_JOIN_INPUT, SCREEN_STATS, SCREEN_TOURNAMENT_DIFF, SCREEN_TIME_ATTACK, SCREEN_GAMEOVER, etc.). "
         "src/features.h provides compile-time feature flags (#define FEATURE_HEATMAP 1, etc.) allowing individual capabilities to be toggled for testing."),
         
        ("4.2 2D Physics & Predictive Trajectory Engine (src/game.c, src/trajectory.c)",
         "src/game.c contains the complete physics simulation. It handles paddle acceleration (8000 px/s²), friction damping, speed clamping (1400 px/s), "
         "boundary enforcement, elastic collisions (restitution = 0.92), and goal scoring. When the paddle strikes the puck with high velocity, it applies a "
         "'flick shot' momentum boost (+0.35 * paddle_speed). Tangential contact imparts angular momentum (spin), which curves the puck using the Magnus effect "
         "(cross-product acceleration perpendicular to velocity). src/trajectory.c raycasts 60 forward steps into the future, computing cushion reflections "
         "to draw a predictive dotted path on the court (toggled with CTRL)."),

        ("4.3 Non-Blocking Cross-Platform Network Layer (src/net.c, src/protocol.h)",
         "src/net.c implements real-time LAN communication using low-level cross-platform TCP sockets on port 50505 (POSIX sockets on Linux/macOS, Winsock2 on Windows). It uses non-blocking I/O to ensure "
         "network I/O never blocks the frame loop. Custom send_all() and recv_all() loops handle TCP stream fragmentation, looping until full byte buffers "
         "are transferred. src/protocol.h defines the Packet struct with message types (MSG_HELLO, MSG_START, MSG_INPUT, MSG_STATE, MSG_BYE). The Host acts "
         "as the authoritative server, accepting client inputs and broadcasting complete world snapshots."),

        ("4.4 Solo Mode Opponent Engine (src/solomode.c)",
         "src/solomode.c implements the Solo Mode autonomous opponent using a 5-state FSM (DEFEND, INTERCEPT, ATTACK, STRIKE, RESET). It features difficulty knobs for reaction delay, "
         "prediction quality, speed caps, and positional jitter. When attacking, it performs a realistic windup step backwards before driving forward at an angle toward "
         "the player's goal. It adapts to player scoring habits by recording player goal zones and dynamically adjusting its defensive resting stance."),

        ("4.5 Procedural Audio & Dynamic Tension (src/audio.c, src/dynaudio.c)",
         "src/audio.c contains a mathematical audio synthesizer that creates 44.1 kHz, 16-bit PCM sound buffers in memory without requiring external .wav or .mp3 files. "
         "Paddle hits, wall thuds, countdown ticks, and fanfare are synthesized using combinations of sine waves, white noise generators, and exponential decay envelopes. "
         "Collision impulse dynamically modulates sound volume and pitch. src/dynaudio.c smoothly ramps audio tension based on current rally length and game pace."),

        ("4.6 Particle Engine & Visual Themes (src/render.c, src/particles.c, src/themes.c)",
         "src/particles.c implements a fixed 512-particle memory pool with zero runtime malloc/free calls to prevent memory leaks and garbage collection stutters. "
         "It generates yellow impact sparks, wall dust, goal confetti explosions, and fading puck motion trails. src/themes.c provides 4 distinct color schemes "
         "(Neon, Retro, Dark, Fire) switchable in real time with the 'T' key. src/render.c coordinates all 2D OpenGL drawing with camera screen shake and goal flash strobes."),

        ("4.7 Gameplay Modifiers & Telemetry (src/powerup.c, src/heatmap.c, src/replay.c, src/tournament.c)",
         "src/powerup.c manages collectible arena tokens (Grow paddle, Slow puck, Freeze opponent) with spawn intervals and active life timers. "
         "src/heatmap.c tracks puck occupancy across a 24x32 grid with exponential decay, rendering a semi-transparent court density map (toggled with TAB). "
         "src/replay.c maintains a 300-frame circular ring buffer (5 seconds at 60 FPS) enabling instant slow-motion goal replays (V key) and disk export (F5). "
         "src/tournament.c manages Best-of-3 and Best-of-5 competitive tournament series tracking."),

        ("4.8 Telemetry & High-Score Persistence (src/stats.c)",
         "src/stats.c monitors real-time match telemetry (rallies, hit conversion accuracy, top puck velocity, hype meter) "
         "and handles binary struct serialization to airhockey.dat. It persists all-time hall-of-fame records "
         "(longest rally, fastest recorded puck shot, maximum goals, lifetime matches) across game restarts with robust fopen/fread/fwrite error validation.")
    ]

    for title, desc in modules_deep:
        add_heading_styled(doc, title, level=2)
        doc.add_paragraph(desc)

    doc.add_page_break()

    # ==================== CHAPTER 5 ====================
    add_heading_styled(doc, "Chapter 5: Project Screenshots & Visual Evidence", level=1)
    doc.add_paragraph(
        "The following screenshots were captured directly from the game engine during live execution. "
        "Each figure illustrates a key architectural subsystem in operation:"
    )

    screens = [
        ("01_main_menu.png", "Figure 5.1: Main Menu & Navigation Interface",
         "The main menu screen rendered at 60 FPS in raylib, providing seamless keyboard and mouse navigation across 8 streamlined game modes (Solo Mode, Local 2-Player, Host LAN, Join LAN, Time Attack, Tournament, AI vs AI Demo, and Quit)."),
        ("02_gameplay_neon.png", "Figure 5.2: Real-Time Gameplay (Neon Theme)",
         "Active match rendering with the high-contrast Neon theme. Demonstrates Player 1 (blue paddle), Player 2 (red paddle), high-speed puck motion, active power-up collectible drops, score counter (3-2), and impact spark particles."),
        ("03_trajectory_prediction.png", "Figure 5.3: Predictive Trajectory Raycasting Engine",
         "Demonstrates the real-time trajectory engine (toggled via CTRL / C) projecting the puck's bank reflections up to 3 seconds into the future using forward raycasting and cushion normal calculations."),
        ("04_heatmap_overlay.png", "Figure 5.4: Territorial Puck Heatmap Density Grid",
         "Real-time analytical heatmap overlay (toggled via TAB). Visualizes a 24x32 grid tracking puck presence density across the rink with exponential decay, revealing offensive and defensive territory control."),
        ("05_match_stats.png", "Figure 5.5: Post-Match Statistics & All-Time Records",
         "Comprehensive post-game telemetry screen displaying rally counters, hit-to-goal conversion accuracy, top recorded puck velocity (1120 px/s), and lifetime high-score records loaded from airhockey.dat."),
        ("06_time_attack.png", "Figure 5.6: Time Attack Challenge Results Screen",
         "Summary screen for the 60-second Time Attack high-score mode. Displays goals scored against the clock, all-time record comparison, and triggers celebratory 'NEW RECORD!' banner notifications."),
        ("07_tournament_mode.png", "Figure 5.7: Tournament Format Selection (Best-of-3 / Best-of-5)",
         "Competitive tournament bracket selector enabling multi-match series play with persistent series score tracking and automated championship verification."),
        ("08_lan_join.png", "Figure 5.8: LAN Multiplayer 4-Digit Room Code Join Interface",
         "The modernized LAN join screen featuring automatic local UDP room beacon discovery and 4-digit Room Code pairing. Eliminates manual IP typing while preserving low-level host-authoritative TCP socket synchronization."),
        ("09_pause_modal.png", "Figure 5.9: In-Game Pause & Navigation Modal Overlay",
         "The in-game pause system triggered seamlessly via ESC, P, or the on-screen HUD button. Displays a centered modal dialog enabling players to Resume Match, Restart Match, or safely Quit to the Main Menu while instantly freezing physics integration and countdown timers."),
        ("10_pause_confirm.png", "Figure 5.10: Action Confirmation Dialog (Restart / Quit Safety Guard)",
         "Two-stage safety confirmation modal preventing accidental match termination. Prompting players to confirm with explicit YES / CANCEL buttons (or ENTER / ESC / Y / N hotkeys) before resetting score progress or returning to the main menu.")
    ]

    base_dir = "/Users/ravichauhan/Desktop/COLLEGE/C/PROJ/screenshots"
    for filename, caption, desc in screens:
        img_path = os.path.join(base_dir, filename)
        add_diagram_figure(doc, img_path, caption, desc, width_inches=4.8)

    doc.add_page_break()

    # ==================== CHAPTER 6 ====================
    add_heading_styled(doc, "Chapter 6: Sample Inputs and Outputs", level=1)
    doc.add_paragraph("The table below details user input actions and the corresponding real-time engine responses:")

    io_table = doc.add_table(rows=14, cols=4)
    io_cols = [Inches(1.4), Inches(1.5), Inches(1.3), Inches(2.3)]
    io_table.rows[0].cells[0].text = "Input Category"
    io_table.rows[0].cells[1].text = "User Action / Key"
    io_table.rows[0].cells[2].text = "Engine State"
    io_table.rows[0].cells[3].text = "System Output / Response"
    io_data = [
        ["Menu Navigation", "UP / DOWN Arrows", "Main Menu", "Cycles highlighted button with audio tick sound."],
        ["Selection", "ENTER Key", "Main Menu", "Transitions to chosen screen with smooth alpha fade."],
        ["Player 1 Paddle", "W, A, S, D", "In-Game", "Applies acceleration vector; clamped to lower half-court."],
        ["Player 2 Paddle", "Arrow Keys", "Local Play", "Applies acceleration vector; clamped to upper half-court."],
        ["Flick Shot", "Fast Paddle Swing", "In-Game", "Imparts 0.35x paddle velocity bonus boost to puck."],
        ["Trajectory Line", "Hold LEFT CTRL", "In-Game", "Raycasts 60 forward steps showing bounce reflections."],
        ["Heatmap Display", "Press TAB", "In-Game", "Toggles 24x32 semi-transparent density overlay."],
        ["Theme Switching", "Press T", "In-Game", "Cycles palette (Neon -> Retro -> Dark -> Fire)."],
        ["Instant Replay", "Press V", "In-Game", "Plays back last 300 frames in 0.35x slow motion."],
        ["Save Replay", "Press F5", "In-Game", "Serializes circular frame buffer to replay.replay."],
        ["Pause / Resume", "Press P / ESC / Click HUD", "In-Game", "Freezes physics loop and summons Match Paused modal overlay."],
        ["Restart / Quit", "Press R or M in Modal", "Paused", "Prompts two-stage safety confirmation modal preventing accidental data loss."],
        ["Confirm Action", "Press ENTER / Y or ESC / N", "Confirmation Modal", "Executes confirmed action or aborts back to pause menu cleanly."]
    ]
    for r_idx, r_data in enumerate(io_data, start=1):
        for c_idx, val in enumerate(r_data):
            io_table.rows[r_idx].cells[c_idx].text = val
    format_table(io_table, io_cols, "1E3A8A")

    add_heading_styled(doc, "6.1 Binary Network Protocol Payloads", level=2)
    doc.add_paragraph("Raw binary socket packets exchanged between Client and Host over port 50505:")
    
    code_p = doc.add_paragraph()
    code_run = code_p.add_run(
        "Client -> Host (MSG_INPUT Packet - 12 Bytes):\n"
        "  [0x00] uint8_t type      = 3 (MSG_INPUT)\n"
        "  [0x04] float   client_tx = 342.50  (Desired X coordinate for client paddle)\n"
        "  [0x08] float   client_ty = 185.00  (Desired Y coordinate for client paddle)\n\n"
        "Host -> Client (MSG_STATE Packet - 56 Bytes):\n"
        "  [0x00] uint8_t type      = 4 (MSG_STATE)\n"
        "  [0x04] float   p1_x, p1_y= 300.0, 700.0  (Host paddle position)\n"
        "  [0x0C] float   p2_x, p2_y= 340.2, 186.1  (Client paddle position)\n"
        "  [0x14] float   puck_x, y = 290.4, 450.2  (Puck coordinates)\n"
        "  [0x1C] float   puck_vx,vy= 410.5,-380.2  (Puck velocity vector)\n"
        "  [0x24] float   puck_spin = 12.4          (Angular momentum)\n"
        "  [0x28] uint8_t score1, 2 = 5, 3          (Authoritative score)\n"
        "  [0x2E] float   goal_flash= 0.0, shake=0.0(Screen effects)"
    )
    code_run.font.name = 'Courier New'
    code_run.font.size = Pt(8.5)
    code_run.font.color.rgb = RGBColor(30, 41, 59)

    add_heading_styled(doc, "6.2 Persistent Disk File Samples", level=2)
    doc.add_paragraph("Sample contents of lifetime hall-of-fame records and binary replay capture files:")
    p_dat = doc.add_paragraph()
    p_dat_run = p_dat.add_run(
        "File: airhockey.dat (All-time hall-of-fame records):\n"
        "  26     <- best_rally_ever (longest volley)\n"
        "  1380.0 <- fastest_puck_ever (px/s)\n"
        "  7      <- most_goals_p1_ever\n"
        "  7      <- most_goals_p2_ever\n"
        "  42     <- matches_played (lifetime matches count)\n\n"
        "File: replay.replay (Binary circular frame buffer export):\n"
        "  [0x00..0x03] uint32_t magic = 0x41484B59 ('AHKY')\n"
        "  [0x04..0x07] uint32_t count = 300 frames (5.0s circular buffer)\n"
        "  [0x08..0x1B] Frame 0 payload (p1x, p1y, p2x, p2y, puck_x, puck_y, spin)\n"
        "  ... 300 sequential telemetry frames at 60 FPS ..."
    )
    p_dat_run.font.name = 'Courier New'
    p_dat_run.font.size = Pt(8.5)
    p_dat_run.font.color.rgb = RGBColor(30, 41, 59)

    # ==================== CHAPTER 7 ====================
    add_heading_styled(doc, "Chapter 7: Hardware & Software Requirements", level=1)
    hw_table = doc.add_table(rows=5, cols=3)
    hw_cols = [Inches(1.8), Inches(2.2), Inches(2.5)]
    hw_table.rows[0].cells[0].text = "Category"
    hw_table.rows[0].cells[1].text = "Minimum Specification"
    hw_table.rows[0].cells[2].text = "Recommended Specification"
    hw_data = [
        ["Operating System", "macOS 12+ / Ubuntu 20.04+ / Linux", "macOS 14+ / Debian-based Linux"],
        ["Processor (CPU)", "Dual-Core 1.8 GHz (x86_64 / ARM64)", "Quad-Core 2.4 GHz or Apple Silicon M1/M2/M3"],
        ["Memory (RAM)", "512 MB Free RAM", "2 GB Free RAM"],
        ["Graphics (GPU)", "OpenGL 2.1 Compatible", "OpenGL 3.3 Core / Metal Accelerated"]
    ]
    for r_idx, r_data in enumerate(hw_data, start=1):
        for c_idx, val in enumerate(r_data):
            hw_table.rows[r_idx].cells[c_idx].text = val
    format_table(hw_table, hw_cols, "1E3A8A")

    # ==================== CHAPTER 8 ====================
    add_heading_styled(doc, "Chapter 8: Conclusion & Future Scope", level=1)
    doc.add_paragraph(
        "This project successfully proves that advanced, production-grade real-time systems can be developed in pure C11 without relying on heavy game engines or external media assets. "
        "By combining non-blocking cross-platform socket programming, continuous sub-stepped physics simulation, an adaptive Solo Mode state-machine opponent, and mathematical procedural waveform synthesis, "
        "the project demonstrates a comprehensive mastery of low-level systems programming, networking, and computer graphics."
    )
    doc.add_paragraph("Future enhancements planned for the project include:")
    future_items = [
        ("NAT Punch-Through: ", "Implementing STUN/UDP relay protocols to allow internet play beyond local subnets."),
        ("4-Player Doubles Mode: ", "Expanding table dimensions and protocol structures to support 2 vs 2 doubles matches."),
        ("Mobile & Gamepad Support: ", "Adding support for dual analog joysticks and touch gesture inputs.")
    ]
    for f_title, f_desc in future_items:
        fp = doc.add_paragraph(style='List Bullet')
        r1 = fp.add_run(f_title)
        r1.bold = True
        r1.font.color.rgb = RGBColor(24, 43, 73)
        fp.add_run(f_desc)

    out_path = "/Users/ravichauhan/Desktop/COLLEGE/C/PROJ/Air_Hockey_Project_Report.docx"
    doc.save(out_path)
    print(f"Successfully generated complete DOCX report at: {out_path}")

if __name__ == "__main__":
    build_report()
