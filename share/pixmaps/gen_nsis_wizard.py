#!/usr/bin/env python3
"""
Generate share/pixmaps/nsis-wizard.bmp with the correct version number.

Usage:
    python3 gen_nsis_wizard.py v2.0.4

The output file is written next to this script as nsis-wizard.bmp.
NSIS MUI2 expects a 164x314 24-bit BMP.
"""
import sys
import os
from PIL import Image, ImageDraw, ImageFont

VERSION = sys.argv[1] if len(sys.argv) > 1 else "v2.0.x"
# Strip leading 'v' for display
VERSION_DISPLAY = VERSION.lstrip("v")

WIDTH, HEIGHT = 164, 314
OUT_PATH = os.path.join(os.path.dirname(__file__), "nsis-wizard.bmp")
ICO_PATH = os.path.join(os.path.dirname(__file__), "bitcoin.ico")

# ── Colours ───────────────────────────────────────────────────────────────────
BG_TOP    = (20,  42,  80)   # deep navy (top)
BG_BOT    = (10,  20,  45)   # near-black navy (bottom)
COIN_RING = (180, 180, 180)  # outer ring
COIN_BG   = (255, 200,  40)  # gold fill
TEXT_H    = (255, 255, 255)  # white headline
TEXT_SUB  = (180, 200, 240)  # soft blue sub-text
TEXT_URL  = (100, 160, 255)  # link blue

# ── Font helpers ──────────────────────────────────────────────────────────────
def _load_font(size, bold=False):
    candidates = []
    if bold:
        candidates = [
            "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
            "C:/Windows/Fonts/arialbd.ttf",
            "C:/Windows/Fonts/calibrib.ttf",
        ]
    else:
        candidates = [
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "C:/Windows/Fonts/arial.ttf",
            "C:/Windows/Fonts/calibri.ttf",
        ]
    for path in candidates:
        if os.path.exists(path):
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()

# ── Background gradient ───────────────────────────────────────────────────────
img = Image.new("RGB", (WIDTH, HEIGHT))
draw = ImageDraw.Draw(img)
for y in range(HEIGHT):
    t = y / HEIGHT
    r = int(BG_TOP[0] + (BG_BOT[0] - BG_TOP[0]) * t)
    g = int(BG_TOP[1] + (BG_BOT[1] - BG_TOP[1]) * t)
    b = int(BG_TOP[2] + (BG_BOT[2] - BG_TOP[2]) * t)
    draw.line([(0, y), (WIDTH, y)], fill=(r, g, b))

# ── Coin logo ─────────────────────────────────────────────────────────────────
COIN_SIZE = 72
COIN_X = (WIDTH - COIN_SIZE) // 2
COIN_Y = 40

if os.path.exists(ICO_PATH):
    try:
        ico = Image.open(ICO_PATH)
        # Pick the largest available size from the ICO
        sizes = ico.info.get("sizes", [])
        best = None
        if sizes:
            largest = max(sizes, key=lambda s: s[0])
            ico.size = largest  # seek to frame
        coin_src = ico.convert("RGBA")
        coin_src = coin_src.resize((COIN_SIZE, COIN_SIZE), Image.LANCZOS)
        # Paste with alpha mask
        img.paste(coin_src, (COIN_X, COIN_Y), coin_src)
    except Exception:
        # Fallback: draw a simple coin placeholder
        _draw_placeholder_coin(draw, COIN_X, COIN_Y, COIN_SIZE)
else:
    # Draw a simple coin placeholder if ICO not found
    cx, cy, r = COIN_X + COIN_SIZE // 2, COIN_Y + COIN_SIZE // 2, COIN_SIZE // 2
    draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=COIN_BG, outline=COIN_RING, width=3)
    font_coin = _load_font(28, bold=True)
    draw.text((cx, cy), "N", fill=(80, 60, 0), font=font_coin, anchor="mm")

# ── Text ──────────────────────────────────────────────────────────────────────
TEXT_Y_START = COIN_Y + COIN_SIZE + 18

font_name  = _load_font(17, bold=True)
font_sub   = _load_font(13, bold=False)
font_ver   = _load_font(14, bold=True)
font_url   = _load_font(11, bold=False)

cx = WIDTH // 2

draw.text((cx, TEXT_Y_START),      "NewYorkCoin",           fill=TEXT_H,   font=font_name, anchor="mt")
draw.text((cx, TEXT_Y_START + 24), "Core",                  fill=TEXT_SUB, font=font_sub,  anchor="mt")
draw.text((cx, TEXT_Y_START + 40), f"v{VERSION_DISPLAY}",  fill=TEXT_H,   font=font_ver,  anchor="mt")

# Thin separator
sep_y = TEXT_Y_START + 64
draw.line([(20, sep_y), (WIDTH - 20, sep_y)], fill=(60, 80, 120), width=1)

draw.text((cx, sep_y + 12), "Open Source",         fill=TEXT_SUB, font=font_url, anchor="mt")
draw.text((cx, sep_y + 26), "Scrypt · AuxPoW",     fill=TEXT_SUB, font=font_url, anchor="mt")

# URL at very bottom
draw.text((cx, HEIGHT - 20), "paywith.nyc", fill=TEXT_URL, font=font_url, anchor="mb")

# ── Save ─────────────────────────────────────────────────────────────────────
# NSIS expects 24-bit BMP (no alpha)
img.convert("RGB").save(OUT_PATH, "BMP")
print(f"Written: {OUT_PATH}  ({WIDTH}x{HEIGHT}, v{VERSION_DISPLAY})")
