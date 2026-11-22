import os
import sys
from docx import Document
from docx.shared import Inches, Pt, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH
from docx.enum.table import WD_TABLE_ALIGNMENT, WD_ALIGN_VERTICAL
from docx.oxml import OxmlElement, parse_xml
from docx.oxml.ns import nsdecls, qn

def set_cell_background(cell, fill_hex):
    shading_elm = parse_xml(f'<w:shd {nsdecls("w")} w:fill="{fill_hex}"/>')
    cell._tc.get_or_add_tcPr().append(shading_elm)

def set_cell_margins(cell, top=80, bottom=80, left=120, right=120):
    tcPr = cell._tc.get_or_add_tcPr()
    tcMar = OxmlElement('w:tcMar')
    for m, val in [('top', top), ('bottom', bottom), ('left', left), ('right', right)]:
        node = OxmlElement(f'w:{m}')
        node.set(qn('w:w'), str(val))
        node.set(qn('w:type'), 'dxa')
        tcMar.append(node)
    tcPr.append(tcMar)

def set_cell_border(cell, **kwargs):
    """
    kwargs: top, bottom, left, right
    values: dict(sz=12, val='single', color='1E3A8A')
    """
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

def build_standalone_cover():
    doc = Document()
    
    # 0.75-inch margins all around for compact single-page layout
    for s in doc.sections:
        s.top_margin = Inches(0.7)
        s.bottom_margin = Inches(0.7)
        s.left_margin = Inches(0.8)
        s.right_margin = Inches(0.8)

    # 1. INSTITUTION HEADER
    p_inst = doc.add_paragraph()
    p_inst.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_inst.paragraph_format.space_before = Pt(0)
    p_inst.paragraph_format.space_after = Pt(2)
    r_inst = p_inst.add_run("[ COLLEGE / UNIVERSITY NAME HERE ]")
    r_inst.font.name = "Calibri"
    r_inst.font.size = Pt(16)
    r_inst.font.bold = True
    r_inst.font.color.rgb = RGBColor(30, 58, 138) # Deep Navy

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
    p_session.paragraph_format.space_after = Pt(12)
    r_ses = p_session.add_run("ACADEMIC YEAR 2026 – 2027")
    r_ses.font.name = "Calibri"
    r_ses.font.size = Pt(9.5)
    r_ses.font.bold = True
    r_ses.font.color.rgb = RGBColor(148, 163, 184)

    # ACCENT DIVIDER (1x1 table acting as colored banner)
    t_rule = doc.add_table(rows=1, cols=1)
    t_rule.alignment = WD_TABLE_ALIGNMENT.CENTER
    cell_rule = t_rule.rows[0].cells[0]
    cell_rule.width = Inches(6.9)
    set_cell_background(cell_rule, "1E3A8A")
    set_cell_margins(cell_rule, top=20, bottom=20, left=0, right=0)
    p_empty = cell_rule.paragraphs[0]
    p_empty.paragraph_format.space_before = Pt(0)
    p_empty.paragraph_format.space_after = Pt(0)
    r_empty = p_empty.add_run()
    r_empty.font.size = Pt(2)

    # 2. TITLE CARD (Card box with subtle border & background)
    p_sp = doc.add_paragraph()
    p_sp.paragraph_format.space_before = Pt(8)
    p_sp.paragraph_format.space_after = Pt(0)

    t_card = doc.add_table(rows=1, cols=1)
    t_card.alignment = WD_TABLE_ALIGNMENT.CENTER
    c_card = t_card.rows[0].cells[0]
    c_card.width = Inches(6.9)
    set_cell_background(c_card, "F8FAFC")
    set_cell_margins(c_card, top=160, bottom=160, left=200, right=200)
    set_cell_border(c_card, 
                    top={'sz': 8, 'val': 'single', 'color': 'CBD5E1'},
                    bottom={'sz': 8, 'val': 'single', 'color': 'CBD5E1'},
                    left={'sz': 18, 'val': 'single', 'color': '1E3A8A'},
                    right={'sz': 8, 'val': 'single', 'color': 'CBD5E1'})

    p_type = c_card.paragraphs[0]
    p_type.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_type.paragraph_format.space_before = Pt(0)
    p_type.paragraph_format.space_after = Pt(4)
    r_type = p_type.add_run("ACADEMIC MINI-PROJECT REPORT")
    r_type.font.name = "Calibri"
    r_type.font.size = Pt(10)
    r_type.font.bold = True
    r_type.font.color.rgb = RGBColor(37, 99, 235) # Accent Blue

    p_t = c_card.add_paragraph()
    p_t.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_t.paragraph_format.space_before = Pt(2)
    p_t.paragraph_format.space_after = Pt(4)
    r_t = p_t.add_run("AIR HOCKEY")
    r_t.font.name = "Calibri"
    r_t.font.size = Pt(26)
    r_t.font.bold = True
    r_t.font.color.rgb = RGBColor(15, 23, 42)

    p_sub = c_card.add_paragraph()
    p_sub.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_sub.paragraph_format.space_before = Pt(0)
    p_sub.paragraph_format.space_after = Pt(8)
    r_sub = p_sub.add_run("A Real-Time 2D LAN Multiplayer & Adaptive Simulation Engine")
    r_sub.font.name = "Calibri"
    r_sub.font.size = Pt(12)
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

    # 3. DEGREE SUBMISSION STATEMENT
    p_deg = doc.add_paragraph()
    p_deg.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_deg.paragraph_format.space_before = Pt(12)
    p_deg.paragraph_format.space_after = Pt(2)
    r_deg1 = p_deg.add_run("Submitted in partial fulfillment of the requirements for the award of the degree of\n")
    r_deg1.font.name = "Calibri"
    r_deg1.font.size = Pt(10.5)
    r_deg1.font.italic = True
    r_deg1.font.color.rgb = RGBColor(51, 65, 85)

    r_deg2 = p_deg.add_run("MASTER OF COMPUTER APPLICATIONS (MCA)")
    r_deg2.font.name = "Calibri"
    r_deg2.font.size = Pt(13)
    r_deg2.font.bold = True
    r_deg2.font.color.rgb = RGBColor(30, 58, 138)

    # 4. TEAM MEMBERS TABLE
    p_team_hdr = doc.add_paragraph()
    p_team_hdr.paragraph_format.space_before = Pt(10)
    p_team_hdr.paragraph_format.space_after = Pt(4)
    r_th = p_team_hdr.add_run("PROJECT TEAM & CONTRIBUTION MATRIX:")
    r_th.font.name = "Calibri"
    r_th.font.size = Pt(10)
    r_th.font.bold = True
    r_th.font.color.rgb = RGBColor(30, 58, 138)

    table = doc.add_table(rows=5, cols=4)
    table.alignment = WD_TABLE_ALIGNMENT.CENTER
    col_widths = [Inches(1.2), Inches(1.8), Inches(1.9), Inches(2.0)]
    headers = ["Roll No.", "Student Name", "Engineering Role", "Assigned Modules"]
    
    header_tr = table.rows[0]
    header_tr._tr.get_or_add_trPr().append(parse_xml(f'<w:tblHeader {nsdecls("w")}/>'))
    for i, h in enumerate(headers):
        cell = header_tr.cells[i]
        set_cell_background(cell, "1E3A8A")
        set_cell_margins(cell, top=70, bottom=70, left=100, right=100)
        cell.width = col_widths[i]
        p = cell.paragraphs[0]
        p.paragraph_format.space_before = Pt(0)
        p.paragraph_format.space_after = Pt(0)
        run = p.add_run(h)
        run.font.name = "Calibri"
        run.font.bold = True
        run.font.color.rgb = RGBColor(255, 255, 255)
        run.font.size = Pt(9.0)

    team_data = [
        ["MCA-2026-01", "Ravi Chauhan (Lead)", "Network & Systems Architect", "main.c, net.c, protocol.h"],
        ["MCA-2026-02", "Team Member 2", "Physics & Simulation Engineer", "game.c, trajectory.c, input.c"],
        ["MCA-2026-03", "Team Member 3", "Graphics & Audio DSP Engineer", "render.c, audio.c, dynaudio.c"],
        ["MCA-2026-04", "Team Member 4", "Solo Mode & Data Systems Engineer", "solomode.c, replay.c, stats.c"]
    ]

    for r_idx, row in enumerate(team_data, start=1):
        bg = "F8FAFC" if r_idx % 2 == 1 else "FFFFFF"
        for c_idx, val in enumerate(row):
            cell = table.rows[r_idx].cells[c_idx]
            set_cell_background(cell, bg)
            set_cell_margins(cell, top=55, bottom=55, left=100, right=100)
            cell.width = col_widths[c_idx]
            p = cell.paragraphs[0]
            p.paragraph_format.space_before = Pt(0)
            p.paragraph_format.space_after = Pt(0)
            run = p.add_run(val)
            run.font.name = "Calibri"
            run.font.size = Pt(8.5)
            if c_idx == 0:
                run.font.bold = True
                run.font.color.rgb = RGBColor(30, 58, 138)
            elif c_idx == 1 and "Ravi" in val:
                run.font.bold = True

    # 5. SUPERVISION & GUIDANCE (2-Column Box)
    p_sp2 = doc.add_paragraph()
    p_sp2.paragraph_format.space_before = Pt(12)
    p_sp2.paragraph_format.space_after = Pt(0)

    t_guide = doc.add_table(rows=1, cols=2)
    t_guide.alignment = WD_TABLE_ALIGNMENT.CENTER
    c_left = t_guide.rows[0].cells[0]
    c_right = t_guide.rows[0].cells[1]
    c_left.width = Inches(3.45)
    c_right.width = Inches(3.45)
    set_cell_background(c_left, "F1F5F9")
    set_cell_background(c_right, "F1F5F9")
    set_cell_margins(c_left, top=100, bottom=100, left=140, right=140)
    set_cell_margins(c_right, top=100, bottom=100, left=140, right=140)
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
    r1.font.size = Pt(9.5)
    r1.font.italic = True
    r1.font.color.rgb = RGBColor(100, 116, 139)

    r2 = p_gl.add_run("[Faculty Guide Name]\n")
    r2.font.name = "Calibri"
    r2.font.size = Pt(11)
    r2.font.bold = True
    r2.font.color.rgb = RGBColor(15, 23, 42)

    r3 = p_gl.add_run("Assistant Professor / Department Guide\nDepartment of Computer Applications")
    r3.font.name = "Calibri"
    r3.font.size = Pt(8.5)
    r3.font.color.rgb = RGBColor(71, 85, 105)

    p_gr = c_right.paragraphs[0]
    p_gr.paragraph_format.space_before = Pt(0)
    p_gr.paragraph_format.space_after = Pt(2)
    r4 = p_gr.add_run("Approved & Endorsed by:\n")
    r4.font.name = "Calibri"
    r4.font.size = Pt(9.5)
    r4.font.italic = True
    r4.font.color.rgb = RGBColor(100, 116, 139)

    r5 = p_gr.add_run("[Head of Department / HOD]\n")
    r5.font.name = "Calibri"
    r5.font.size = Pt(11)
    r5.font.bold = True
    r5.font.color.rgb = RGBColor(15, 23, 42)

    r6 = p_gr.add_run("Professor & Head of Department\nDepartment of Computer Applications")
    r6.font.name = "Calibri"
    r6.font.size = Pt(8.5)
    r6.font.color.rgb = RGBColor(71, 85, 105)

    # 6. FOOTER
    p_foot = doc.add_paragraph()
    p_foot.alignment = WD_ALIGN_PARAGRAPH.CENTER
    p_foot.paragraph_format.space_before = Pt(14)
    p_foot.paragraph_format.space_after = Pt(0)
    r_foot = p_foot.add_run("DEPARTMENT OF COMPUTER APPLICATIONS  •  NOVEMBER 2026\n[City, State, India]")
    r_foot.font.name = "Calibri"
    r_foot.font.size = Pt(9.0)
    r_foot.font.bold = True
    r_foot.font.color.rgb = RGBColor(148, 163, 184)

    out_path = "Air_Hockey_Cover_Page.docx"
    doc.save(out_path)
    print("Successfully generated standalone cover page DOCX at:", os.path.abspath(out_path))

def build_standalone_html_cover():
    html = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Air Hockey Project Report - Cover Page</title>
    <style>
        @page {
            size: A4 portrait;
            margin: 0;
        }
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }
        body {
            font-family: 'Segoe UI', -apple-system, BlinkMacSystemFont, Roboto, 'Helvetica Neue', Arial, sans-serif;
            background: #e2e8f0;
            color: #1e293b;
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            padding: 24px;
        }
        .page {
            width: 210mm;
            min-height: 297mm;
            padding: 16mm 18mm;
            background: #ffffff;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.15);
            position: relative;
            display: flex;
            flex-direction: column;
            justify-content: space-between;
        }
        .outer-border {
            border: 3px double #1e3a8a;
            padding: 12mm 14mm;
            height: 100%;
            display: flex;
            flex-direction: column;
            justify-content: space-between;
            position: relative;
        }
        .outer-border::before {
            content: "";
            position: absolute;
            top: 4px; left: 4px; right: 4px; bottom: 4px;
            border: 1px solid #cbd5e1;
            pointer-events: none;
        }
        
        /* Header */
        .institution-header {
            text-align: center;
            margin-bottom: 12px;
        }
        .college-name {
            font-size: 20px;
            font-weight: 800;
            color: #1e3a8a;
            text-transform: uppercase;
            letter-spacing: 0.8px;
            margin-bottom: 4px;
        }
        .college-sub {
            font-size: 11px;
            color: #64748b;
            font-style: italic;
            margin-bottom: 6px;
        }
        .dept-title {
            font-size: 13px;
            font-weight: 700;
            color: #0f172a;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        .academic-year {
            font-size: 11px;
            font-weight: 600;
            color: #94a3b8;
            margin-top: 3px;
        }

        .header-divider {
            height: 3px;
            background: linear-gradient(90deg, #1e3a8a 0%, #3b82f6 50%, #1e3a8a 100%);
            margin: 10px 0 16px 0;
            border-radius: 2px;
        }

        /* Title Card */
        .title-card {
            background: #f8fafc;
            border: 1px solid #cbd5e1;
            border-left: 6px solid #1e3a8a;
            border-radius: 8px;
            padding: 18px 20px;
            text-align: center;
            margin-bottom: 16px;
        }
        .project-tag {
            font-size: 11px;
            font-weight: 700;
            color: #2563eb;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 6px;
        }
        .main-title {
            font-size: 32px;
            font-weight: 900;
            color: #0f172a;
            letter-spacing: 1px;
            margin-bottom: 6px;
        }
        .subtitle {
            font-size: 14px;
            font-weight: 600;
            color: #1e3a8a;
            margin-bottom: 12px;
        }
        .tech-badges {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: 8px;
        }
        .badge {
            font-size: 10px;
            font-weight: 600;
            color: #334155;
            background: #ffffff;
            border: 1px solid #cbd5e1;
            padding: 3px 8px;
            border-radius: 12px;
        }

        /* Degree Submission */
        .submission-clause {
            text-align: center;
            margin-bottom: 16px;
        }
        .clause-text {
            font-size: 12px;
            color: #475569;
            font-style: italic;
            margin-bottom: 4px;
        }
        .degree-name {
            font-size: 15px;
            font-weight: 800;
            color: #1e3a8a;
            letter-spacing: 0.5px;
        }

        /* Team Table */
        .section-heading {
            font-size: 11px;
            font-weight: 700;
            color: #1e3a8a;
            text-transform: uppercase;
            letter-spacing: 0.5px;
            margin-bottom: 6px;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin-bottom: 16px;
            font-size: 11px;
        }
        th {
            background: #1e3a8a;
            color: #ffffff;
            font-weight: 700;
            text-align: left;
            padding: 7px 10px;
            font-size: 10.5px;
        }
        td {
            padding: 6px 10px;
            border-bottom: 1px solid #e2e8f0;
            color: #334155;
        }
        tr:nth-child(even) {
            background: #f8fafc;
        }
        .roll-no {
            font-weight: 700;
            color: #1e3a8a;
            font-family: monospace;
            font-size: 11px;
        }
        .lead-name {
            font-weight: 700;
            color: #0f172a;
        }
        .code-unit {
            font-family: 'Consolas', monospace;
            font-size: 10px;
            background: #f1f5f9;
            padding: 1px 4px;
            border-radius: 3px;
            border: 1px solid #e2e8f0;
        }

        /* Guidance & Approval Grid */
        .approval-grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 16px;
            margin-bottom: 16px;
        }
        .approval-box {
            background: #f8fafc;
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            padding: 10px 14px;
        }
        .approval-role {
            font-size: 10px;
            font-style: italic;
            color: #64748b;
            margin-bottom: 4px;
        }
        .approval-name {
            font-size: 12px;
            font-weight: 700;
            color: #0f172a;
            margin-bottom: 2px;
        }
        .approval-desc {
            font-size: 10px;
            color: #475569;
        }
        .sig-space {
            height: 28px;
            margin-top: 10px;
            border-bottom: 1px dashed #94a3b8;
            position: relative;
        }
        .sig-label {
            position: absolute;
            bottom: 2px;
            right: 0;
            font-size: 9px;
            color: #94a3b8;
            font-style: italic;
        }

        /* Footer */
        .footer {
            text-align: center;
            border-top: 1px solid #cbd5e1;
            padding-top: 10px;
            font-size: 10px;
            font-weight: 600;
            color: #64748b;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }

        /* Print Button (Hidden on Print) */
        .print-bar {
            position: fixed;
            bottom: 20px;
            background: #0f172a;
            color: #ffffff;
            padding: 12px 24px;
            border-radius: 30px;
            box-shadow: 0 4px 15px rgba(0,0,0,0.3);
            display: flex;
            align-items: center;
            gap: 14px;
            font-size: 14px;
            z-index: 1000;
        }
        .print-btn {
            background: #2563eb;
            color: white;
            border: none;
            padding: 6px 14px;
            border-radius: 20px;
            font-weight: 700;
            cursor: pointer;
            transition: 0.2s;
        }
        .print-btn:hover {
            background: #1d4ed8;
        }

        @media print {
            body {
                background: none;
                padding: 0;
            }
            .page {
                box-shadow: none;
                width: 100%;
                min-height: 100%;
                padding: 14mm;
            }
            .print-bar {
                display: none !important;
            }
        }
    </style>
</head>
<body>

    <div class="print-bar">
        <span>Academic Cover Page Ready</span>
        <button class="print-btn" onclick="window.print()">Print / Save as PDF</button>
    </div>

    <div class="page">
        <div class="outer-border">
            <!-- Institution Header -->
            <div class="institution-header">
                <div class="college-name">[ COLLEGE / UNIVERSITY NAME HERE ]</div>
                <div class="college-sub">Approved by AICTE, New Delhi &bull; Affiliated to [University Name]</div>
                <div class="dept-title">Department of Computer Applications</div>
                <div class="academic-year">Academic Year 2026 – 2027</div>
            </div>

            <div class="header-divider"></div>

            <!-- Title Card -->
            <div class="title-card">
                <div class="project-tag">Major Project Report</div>
                <div class="main-title">AIR HOCKEY</div>
                <div class="subtitle">A Real-Time 2D LAN Multiplayer & Adaptive Simulation Engine</div>
                <div class="tech-badges">
                    <span class="badge">Pure C11</span>
                    <span class="badge">OpenGL / Raylib</span>
                    <span class="badge">Raw TCP Sockets</span>
                    <span class="badge">Procedural DSP Audio</span>
                    <span class="badge">Sub-Stepped Physics</span>
                </div>
            </div>

            <!-- Degree Submission Clause -->
            <div class="submission-clause">
                <div class="clause-text">A project report submitted in partial fulfillment of the requirements for the award of the degree of</div>
                <div class="degree-name">MASTER OF COMPUTER APPLICATIONS (MCA)</div>
            </div>

            <!-- Team Members Table -->
            <div>
                <div class="section-heading">Project Team & Contribution Matrix:</div>
                <table>
                    <thead>
                        <tr>
                            <th style="width: 18%;">Roll No.</th>
                            <th style="width: 28%;">Student Name</th>
                            <th style="width: 27%;">Engineering Role</th>
                            <th style="width: 27%;">Assigned Modules</th>
                        </tr>
                    </thead>
                    <tbody>
                        <tr>
                            <td class="roll-no">MCA-2026-01</td>
                            <td class="lead-name">Ravi Chauhan (Lead)</td>
                            <td>Network & Systems Architect</td>
                            <td><span class="code-unit">main.c</span> <span class="code-unit">net.c</span> <span class="code-unit">protocol.h</span></td>
                        </tr>
                        <tr>
                            <td class="roll-no">MCA-2026-02</td>
                            <td>Team Member 2</td>
                            <td>Physics & Simulation Engineer</td>
                            <td><span class="code-unit">game.c</span> <span class="code-unit">trajectory.c</span> <span class="code-unit">input.c</span></td>
                        </tr>
                        <tr>
                            <td class="roll-no">MCA-2026-03</td>
                            <td>Team Member 3</td>
                            <td>Graphics & Audio DSP Engineer</td>
                            <td><span class="code-unit">render.c</span> <span class="code-unit">audio.c</span> <span class="code-unit">dynaudio.c</span></td>
                        </tr>
                        <tr>
                            <td class="roll-no">MCA-2026-04</td>
                            <td>Team Member 4</td>
                            <td>Solo Mode & Data Systems Engineer</td>
                            <td><span class="code-unit">solomode.c</span> <span class="code-unit">replay.c</span> <span class="code-unit">stats.c</span></td>
                        </tr>
                    </tbody>
                </table>
            </div>

            <!-- Supervision & Approval Grid -->
            <div class="approval-grid">
                <div class="approval-box">
                    <div class="approval-role">Under the Guidance of:</div>
                    <div class="approval-name">[Faculty Guide Name]</div>
                    <div class="approval-desc">Assistant Professor, Dept. of Computer Applications</div>
                    <div class="sig-space">
                        <span class="sig-label">Signature of Faculty Guide</span>
                    </div>
                </div>
                <div class="approval-box">
                    <div class="approval-role">Approved & Endorsed by:</div>
                    <div class="approval-name">[Head of Department / HOD]</div>
                    <div class="approval-desc">Professor & Head, Dept. of Computer Applications</div>
                    <div class="sig-space">
                        <span class="sig-label">Signature of HOD</span>
                    </div>
                </div>
            </div>

            <!-- Footer -->
            <div class="footer">
                Department of Computer Applications &bull; [City, State, India] &bull; November 2026
            </div>
        </div>
    </div>

</body>
</html>
"""
    out_path = "Air_Hockey_Cover_Page.html"
    with open(out_path, "w", encoding="utf-8") as f:
        f.write(html)
    print("Successfully generated standalone cover page HTML at:", os.path.abspath(out_path))

if __name__ == '__main__':
    build_standalone_cover()
    build_standalone_html_cover()


