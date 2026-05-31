"""Generate Pokemon-themed board tile and frame PNGs for GUI/Board/."""
from pathlib import Path
from PIL import Image, ImageDraw

OUT = Path(__file__).resolve().parent.parent / "GUI" / "Board"
OUT.mkdir(parents=True, exist_ok=True)

CELL = 64
FRAME_PAD = 22

PK_RED = (238, 21, 21)
PK_RED_D = (180, 16, 16)
PK_WHITE = (248, 248, 248)
PK_BLACK = (32, 32, 40)
PK_GOLD = (255, 203, 5)
PK_BLUE = (52, 120, 220)
PK_GRASS_L = (120, 192, 72)
PK_GRASS_D = (72, 152, 48)
PK_CAVE_L = (108, 116, 140)
PK_CAVE_D = (72, 80, 104)
PK_SAND_L = (248, 224, 168)
PK_SAND_D = (216, 184, 120)
ROUTE_TAN = (200, 168, 96)


def draw_pokeball_icon(d, cx, cy, r):
    d.ellipse([cx - r, cy - r, cx + r, cy + r], fill=PK_WHITE, outline=PK_BLACK, width=2)
    d.rectangle([cx - r, cy - 2, cx + r, cy + 2], fill=PK_BLACK)
    d.pieslice([cx - r, cy - r, cx + r, cy + r], 180, 360, fill=PK_RED)
    d.ellipse([cx - r // 3, cy - r // 3, cx + r // 3, cy + r // 3], fill=PK_WHITE, outline=PK_BLACK, width=1)


def grass_cell(base, dark, accent, is_light):
    img = Image.new("RGBA", (CELL, CELL), base + (255,))
    d = ImageDraw.Draw(img)

    # Route-tile cross pattern (Pokemon overworld feel)
    route = ROUTE_TAN if is_light else (ROUTE_TAN[0] - 20, ROUTE_TAN[1] - 16, ROUTE_TAN[2] - 12)
    d.rectangle([CELL // 2 - 5, 0, CELL // 2 + 4, CELL - 1], fill=route + (255,))
    d.rectangle([0, CELL // 2 - 5, CELL - 1, CELL // 2 + 4], fill=route + (255,))

    # Grass tufts
    for i in range(6):
        gx = 4 + i * 10 + (i % 2) * 3
        gy = CELL - 8 - (i % 3) * 2
        d.line([(gx, gy), (gx + 2, gy - 10)], fill=accent, width=2)
        d.line([(gx + 4, gy + 1), (gx + 6, gy - 8)], fill=dark + (255,), width=2)

    # Tiny flowers
    if is_light:
        for fx, fy in [(12, 18), (44, 28), (28, 46)]:
            d.ellipse([fx, fy, fx + 3, fy + 3], fill=(255, 180, 200, 255))
            d.point((fx + 1, fy + 1), fill=(255, 240, 80, 255))

    # Grid bevel
    d.rectangle([0, 0, CELL - 1, CELL - 1], outline=accent, width=1)
    d.line([(1, 1), (CELL - 2, 1)], fill=(255, 255, 255, 60))
    d.line([(1, 1), (1, CELL - 2)], fill=(255, 255, 255, 50))
    return img


def cave_cell(base, dark, accent, is_light):
    img = Image.new("RGBA", (CELL, CELL), base + (255,))
    d = ImageDraw.Draw(img)

    # Rock cracks
    for i in range(3):
        x0 = 8 + i * 18
        d.line([(x0, 6), (x0 + 8, CELL - 8)], fill=dark + (255,), width=2)
        d.line([(x0 + 2, 10), (x0 + 10, CELL - 4)], fill=accent, width=1)

    # Crystals
    for i in range(2):
        cx = 16 + i * 32
        cy = 20 + (i % 2) * 12
        pts = [(cx, cy - 10), (cx - 6, cy + 6), (cx + 6, cy + 6)]
        col = (140, 200, 255, 255) if is_light else (100, 160, 220, 255)
        d.polygon(pts, fill=col, outline=(60, 100, 160, 255))

    d.rectangle([0, 0, CELL - 1, CELL - 1], outline=accent, width=1)
    return img


def arena_cell(base, dark, accent, is_light):
    img = Image.new("RGBA", (CELL, CELL), base + (255,))
    d = ImageDraw.Draw(img)

    # Stadium floor stripes
    stripe = accent if is_light else dark + (255,)
    for i in range(0, CELL, 12):
        d.line([(i, 0), (i, CELL - 1)], fill=stripe, width=1)
    d.line([(0, CELL // 2), (CELL - 1, CELL // 2)], fill=PK_GOLD + (255,), width=2)

    # Center circle hint on some tiles
    if is_light:
        d.ellipse([CELL // 2 - 8, CELL // 2 - 8, CELL // 2 + 8, CELL // 2 + 8],
                  outline=PK_RED_D + (255,), width=1)

    d.rectangle([0, 0, CELL - 1, CELL - 1], outline=PK_GOLD + (255,), width=1)
    return img


def poke_cell(base, dark, accent, pattern, is_light):
    if pattern == "grass":
        return grass_cell(base, dark, accent, is_light)
    if pattern == "cave":
        return cave_cell(base, dark, accent, is_light)
    return arena_cell(base, dark, accent, is_light)


def make_frame(name, outer, inner, trim, theme):
    inner_px = CELL * 12
    total = inner_px + FRAME_PAD * 2
    frame = Image.new("RGBA", (total, total), (0, 0, 0, 0))
    d = ImageDraw.Draw(frame)

    # Stadium outer shell
    d.rounded_rectangle([0, 0, total - 1, total - 1], radius=18, fill=outer, outline=inner, width=6)
    d.rounded_rectangle([6, 6, total - 7, total - 7], radius=16, outline=trim, width=3)

    # Hollow play area
    d.rounded_rectangle(
        [FRAME_PAD, FRAME_PAD, total - FRAME_PAD - 1, total - FRAME_PAD - 1],
        radius=8, fill=(0, 0, 0, 0),
    )
    d.rounded_rectangle(
        [FRAME_PAD, FRAME_PAD, total - FRAME_PAD - 1, total - FRAME_PAD - 1],
        radius=8, outline=trim, width=2,
    )

    # Top battle banner bar
    bar_h = FRAME_PAD - 4
    d.rounded_rectangle([FRAME_PAD, 4, total - FRAME_PAD, 4 + bar_h], radius=6, fill=PK_RED)
    d.rounded_rectangle([FRAME_PAD + 2, 6, total - FRAME_PAD - 2, 4 + bar_h - 2], radius=5, fill=PK_RED_D)
    mid = total // 2
    draw_pokeball_icon(d, mid, 4 + bar_h // 2 + 1, 9)

    # Corner pokeballs
    for px, py in [(FRAME_PAD // 2 + 8, FRAME_PAD // 2 + 8),
                   (total - FRAME_PAD // 2 - 8, FRAME_PAD // 2 + 8),
                   (FRAME_PAD // 2 + 8, total - FRAME_PAD // 2 - 8),
                   (total - FRAME_PAD // 2 - 8, total - FRAME_PAD // 2 - 8)]:
        draw_pokeball_icon(d, px, py, 11)

    # Theme accents along inner edge
    if theme == "grass":
        for i in range(12):
            x = FRAME_PAD + i * (inner_px // 12)
            d.rectangle([x, FRAME_PAD - 2, x + 6, FRAME_PAD + 1], fill=PK_GRASS_D + (255,))
    elif theme == "cave":
        for i in range(8):
            x = FRAME_PAD + 8 + i * (inner_px // 8)
            d.polygon([(x, FRAME_PAD - 1), (x + 4, FRAME_PAD + 5), (x + 8, FRAME_PAD - 1)],
                      fill=PK_BLUE + (255,))
    else:
        for i in range(6):
            x = FRAME_PAD + 12 + i * (inner_px // 6)
            d.ellipse([x, FRAME_PAD - 3, x + 8, FRAME_PAD + 5], fill=PK_GOLD + (255,))

    frame.save(OUT / f"board_frame_{name}.png")


def make_theme(name, light, dark, accent, pattern, frame_outer, frame_inner, frame_trim):
    poke_cell(light, dark, accent, pattern, True).save(OUT / f"cell_light_{name}.png")
    poke_cell(dark, light, accent, pattern, False).save(OUT / f"cell_dark_{name}.png")
    make_frame(name, frame_outer, frame_inner, frame_trim, pattern)


make_theme("wood", PK_GRASS_L, PK_GRASS_D, (56, 128, 40), "grass", PK_RED, PK_RED_D, PK_GOLD)
make_theme("slate", PK_CAVE_L, PK_CAVE_D, (160, 200, 255), "cave", (40, 44, 64), PK_BLUE, (180, 210, 255))
make_theme("gold", PK_SAND_L, PK_SAND_D, (200, 150, 64), "sand", PK_RED_D, PK_GOLD, PK_WHITE)

print(f"Generated Pokemon-themed board assets in {OUT}")
