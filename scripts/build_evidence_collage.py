from pathlib import Path

from PIL import Image, ImageDraw, ImageFont, ImageOps


ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "assets"
OUTPUT = ASSETS / "四灯流水实物验证.jpg"

ITEMS = [
    ("PA8点亮.jpg", "PA8 外接 LED 点亮"),
    ("PA15点亮.jpg", "PA15 外接 LED 点亮"),
    ("PB8点亮.jpg", "PB8 外接 LED 点亮"),
    ("PC13板载灯点亮.jpg", "PC13 板载 LED 点亮"),
]

PANEL_SIZE = (1000, 750)
LABEL_HEIGHT = 90
CANVAS_SIZE = (PANEL_SIZE[0] * 2, (PANEL_SIZE[1] + LABEL_HEIGHT) * 2)


def load_font(size: int) -> ImageFont.FreeTypeFont:
    for path in (
        Path(r"C:\Windows\Fonts\msyhbd.ttc"),
        Path(r"C:\Windows\Fonts\simhei.ttf"),
    ):
        if path.exists():
            return ImageFont.truetype(str(path), size)
    raise RuntimeError("未找到可用中文字体")


def main() -> None:
    canvas = Image.new("RGB", CANVAS_SIZE, "white")
    draw = ImageDraw.Draw(canvas)
    font = load_font(42)

    for index, (filename, label) in enumerate(ITEMS):
        row, column = divmod(index, 2)
        x = column * PANEL_SIZE[0]
        y = row * (PANEL_SIZE[1] + LABEL_HEIGHT)

        with Image.open(ASSETS / filename) as source:
            source = ImageOps.exif_transpose(source).convert("RGB")
            panel = ImageOps.fit(source, PANEL_SIZE, method=Image.Resampling.LANCZOS)
        canvas.paste(panel, (x, y))

        label_y = y + PANEL_SIZE[1]
        draw.rectangle((x, label_y, x + PANEL_SIZE[0], label_y + LABEL_HEIGHT), fill="#0B3A57")
        bbox = draw.textbbox((0, 0), label, font=font)
        text_x = x + (PANEL_SIZE[0] - (bbox[2] - bbox[0])) // 2
        text_y = label_y + (LABEL_HEIGHT - (bbox[3] - bbox[1])) // 2 - bbox[1]
        draw.text((text_x, text_y), label, fill="white", font=font)

    canvas.save(OUTPUT, quality=92, optimize=True)
    print(OUTPUT)


if __name__ == "__main__":
    main()
