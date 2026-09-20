from __future__ import annotations

import re
from pathlib import Path

from reportlab.lib import colors
from reportlab.lib.enums import TA_CENTER, TA_LEFT
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle, getSampleStyleSheet
from reportlab.lib.units import mm
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.platypus import (
    Image,
    PageBreak,
    Paragraph,
    Preformatted,
    SimpleDocTemplate,
    Spacer,
    Table,
    TableStyle,
)


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "实验报告.md"
OUTPUT = ROOT / "output" / "STM32F103C8T6_BluePill_流水灯实验报告.pdf"


def register_fonts() -> tuple[str, str, str]:
    candidates = [
        (
            Path(r"C:\Windows\Fonts\msyh.ttc"),
            Path(r"C:\Windows\Fonts\msyhbd.ttc"),
            Path(r"C:\Windows\Fonts\consola.ttf"),
        ),
        (
            Path(r"C:\Windows\Fonts\simhei.ttf"),
            Path(r"C:\Windows\Fonts\simhei.ttf"),
            Path(r"C:\Windows\Fonts\consola.ttf"),
        ),
    ]
    for regular, bold, mono in candidates:
        if regular.exists() and bold.exists():
            pdfmetrics.registerFont(TTFont("CN", str(regular)))
            pdfmetrics.registerFont(TTFont("CN-Bold", str(bold)))
            # Use the CJK-capable face for code as well. Consolas cannot render
            # Chinese comments or labels and would silently drop glyphs.
            pdfmetrics.registerFont(TTFont("Code", str(regular)))
            return "CN", "CN-Bold", "Code"
    raise RuntimeError("No suitable Chinese font found in C:\\Windows\\Fonts")


REGULAR, BOLD, MONO = register_fonts()


def clean_inline(text: str) -> str:
    text = text.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
    text = re.sub(r"!\[([^]]*)\]\(([^)]+)\)", r"\1", text)
    text = re.sub(r"\[([^]]+)\]\(([^)]+)\)", r'<link href="\2" color="#1B5E8A">\1</link>', text)
    text = re.sub(r"`([^`]+)`", r'<font name="Code">\1</font>', text)
    text = re.sub(r"\*\*([^*]+)\*\*", r"<b>\1</b>", text)
    return text


def make_styles():
    styles = getSampleStyleSheet()
    body = ParagraphStyle(
        "BodyCN",
        parent=styles["BodyText"],
        fontName=REGULAR,
        fontSize=9.5,
        leading=15,
        textColor=colors.HexColor("#24313A"),
        alignment=TA_LEFT,
        wordWrap="CJK",
        spaceAfter=5,
    )
    styles.add(body)
    styles.add(
        ParagraphStyle(
            "TitleCN",
            parent=body,
            fontName=BOLD,
            fontSize=22,
            leading=30,
            alignment=TA_CENTER,
            textColor=colors.HexColor("#123B5D"),
            spaceAfter=18,
        )
    )
    for level, size, color, before, after in [
        (1, 16, "#123B5D", 14, 8),
        (2, 13, "#176B87", 11, 6),
        (3, 11, "#2C5364", 8, 4),
    ]:
        styles.add(
            ParagraphStyle(
                f"H{level}CN",
                parent=body,
                fontName=BOLD,
                fontSize=size,
                leading=size + 6,
                textColor=colors.HexColor(color),
                spaceBefore=before,
                spaceAfter=after,
                keepWithNext=True,
            )
        )
    styles.add(
        ParagraphStyle(
            "QuoteCN",
            parent=body,
            leftIndent=10,
            rightIndent=8,
            borderColor=colors.HexColor("#48A9A6"),
            borderWidth=1,
            borderPadding=7,
            backColor=colors.HexColor("#EDF8F7"),
            textColor=colors.HexColor("#244A4A"),
        )
    )
    styles.add(
        ParagraphStyle(
            "CodeCN",
            parent=body,
            fontName=MONO,
            fontSize=7.3,
            leading=10.2,
            leftIndent=5,
            rightIndent=5,
            borderPadding=7,
            backColor=colors.HexColor("#F3F5F7"),
            borderColor=colors.HexColor("#CFD8DC"),
            borderWidth=0.5,
            wordWrap="CJK",
        )
    )
    styles.add(
        ParagraphStyle(
            "ListCN",
            parent=body,
            leftIndent=14,
            firstLineIndent=-9,
            bulletIndent=4,
        )
    )
    styles.add(
        ParagraphStyle(
            "TableCN",
            parent=body,
            fontSize=7.5,
            leading=10.5,
            wordWrap="CJK",
            spaceAfter=0,
        )
    )
    return styles


STYLES = make_styles()


def add_table(story, rows: list[list[str]]) -> None:
    if not rows:
        return
    col_count = max(len(row) for row in rows)
    normalized = [row + [""] * (col_count - len(row)) for row in rows]
    data = [
        [Paragraph(clean_inline(cell.strip()), STYLES["TableCN"]) for cell in row]
        for row in normalized
    ]
    usable_width = A4[0] - 30 * mm
    widths = [usable_width / col_count] * col_count
    table = Table(data, colWidths=widths, repeatRows=1, hAlign="LEFT")
    table.setStyle(
        TableStyle(
            [
                ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#176B87")),
                ("TEXTCOLOR", (0, 0), (-1, 0), colors.white),
                ("FONTNAME", (0, 0), (-1, 0), BOLD),
                ("BACKGROUND", (0, 1), (-1, -1), colors.HexColor("#F7FAFC")),
                ("GRID", (0, 0), (-1, -1), 0.35, colors.HexColor("#AAB7C0")),
                ("VALIGN", (0, 0), (-1, -1), "MIDDLE"),
                ("LEFTPADDING", (0, 0), (-1, -1), 4),
                ("RIGHTPADDING", (0, 0), (-1, -1), 4),
                ("TOPPADDING", (0, 0), (-1, -1), 4),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 4),
            ]
        )
    )
    story.extend([table, Spacer(1, 6)])


def parse_markdown() -> list:
    lines = SOURCE.read_text(encoding="utf-8").splitlines()
    story: list = []
    paragraph_buffer: list[str] = []
    table_rows: list[list[str]] = []
    code_lines: list[str] = []
    in_code = False
    first_title = True

    def flush_paragraph() -> None:
        nonlocal paragraph_buffer
        if paragraph_buffer:
            text = " ".join(part.strip() for part in paragraph_buffer)
            story.append(Paragraph(clean_inline(text), STYLES["BodyCN"]))
            paragraph_buffer = []

    def flush_table() -> None:
        nonlocal table_rows
        if table_rows:
            rows = [row for row in table_rows if not all(re.fullmatch(r":?-{3,}:?", c.strip()) for c in row)]
            add_table(story, rows)
            table_rows = []

    for line in lines:
        stripped = line.strip()

        if stripped.startswith("```"):
            flush_paragraph()
            flush_table()
            if in_code:
                story.extend([Preformatted("\n".join(code_lines), STYLES["CodeCN"]), Spacer(1, 5)])
                code_lines = []
                in_code = False
            else:
                in_code = True
            continue

        if in_code:
            code_lines.append(line.expandtabs(4))
            continue

        image_match = re.fullmatch(r"!\[([^]]*)\]\(([^)]+)\)", stripped)
        if image_match:
            flush_paragraph()
            flush_table()
            image_path = ROOT / image_match.group(2)
            img = Image(str(image_path))
            max_w, max_h = 165 * mm, 190 * mm
            scale = min(max_w / img.imageWidth, max_h / img.imageHeight)
            img.drawWidth = img.imageWidth * scale
            img.drawHeight = img.imageHeight * scale
            img.hAlign = "CENTER"
            story.extend(
                [img, Spacer(1, 4), Paragraph(image_match.group(1), STYLES["BodyCN"]), Spacer(1, 6)]
            )
            continue

        if stripped.startswith("|") and stripped.endswith("|"):
            flush_paragraph()
            table_rows.append([cell.strip() for cell in stripped[1:-1].split("|")])
            continue
        flush_table()

        heading_match = re.match(r"^(#{1,3})\s+(.+)$", stripped)
        if heading_match:
            flush_paragraph()
            level = len(heading_match.group(1))
            text = heading_match.group(2)
            if first_title and level == 1:
                story.append(Spacer(1, 12 * mm))
                story.append(Paragraph(clean_inline(text), STYLES["TitleCN"]))
                story.append(Spacer(1, 4 * mm))
                first_title = False
            else:
                story.append(Paragraph(clean_inline(text), STYLES[f"H{level}CN"]))
            continue

        if stripped.startswith("> "):
            flush_paragraph()
            story.append(Paragraph(clean_inline(stripped[2:]), STYLES["QuoteCN"]))
            story.append(Spacer(1, 5))
            continue

        list_match = re.match(r"^(?:[-*]|\d+\.)\s+(.+)$", stripped)
        if list_match:
            flush_paragraph()
            bullet = "•" if stripped[0] in "-*" else stripped.split()[0]
            story.append(
                Paragraph(f"{bullet} {clean_inline(list_match.group(1))}", STYLES["ListCN"])
            )
            continue

        if not stripped:
            flush_paragraph()
            continue

        paragraph_buffer.append(stripped)

    flush_paragraph()
    flush_table()
    return story


def draw_page(canvas, doc) -> None:
    canvas.saveState()
    width, height = A4
    canvas.setStrokeColor(colors.HexColor("#D9E2E8"))
    canvas.setLineWidth(0.5)
    canvas.line(15 * mm, 13 * mm, width - 15 * mm, 13 * mm)
    canvas.setFont(REGULAR, 7.5)
    canvas.setFillColor(colors.HexColor("#607D8B"))
    canvas.drawString(15 * mm, 8 * mm, "STM32F103C8T6 Blue Pill 裸寄存器流水灯实验报告")
    canvas.drawRightString(width - 15 * mm, 8 * mm, f"第 {doc.page} 页")
    canvas.restoreState()


def main() -> None:
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    document = SimpleDocTemplate(
        str(OUTPUT),
        pagesize=A4,
        rightMargin=15 * mm,
        leftMargin=15 * mm,
        topMargin=15 * mm,
        bottomMargin=18 * mm,
        title="STM32F103C8T6 Blue Pill 裸寄存器流水灯实验报告",
        author="",
        subject="GPIO register-level LED chaser laboratory report",
    )
    document.build(parse_markdown(), onFirstPage=draw_page, onLaterPages=draw_page)
    print(OUTPUT)


if __name__ == "__main__":
    main()
