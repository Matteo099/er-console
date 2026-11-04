#!/usr/bin/env python3
"""
Builds the UI variant of "Escaperiom Display" from the ESCAPERIOM logo geometry.
Lighter strokes, narrower spacing and Latin-1 accents improve small headings.

Glyphs: A-Z, 0-9 and common symbols. Lowercase maps to the same glyphs.
'o' (lowercase) = plain O without the puzzle-core square; 'O' keeps the core.

    pip install fonttools skia-pathops brotli
    python scripts/fonts/make_escaperiom_font.py  # -> assets/fonts/*.ttf, *.woff2

Then for LVGL:
    lv_font_conv --font EscaperiomDisplay.ttf --size 32 --bpp 4 \
        --range 0x20,0x41-0x5A,0x61-0x7A --format lvgl -o font_escaperiom_32.c
"""
import argparse
import math
from pathlib import Path
import unicodedata
from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen

# ---- design space (identical to the logo) ---------------------------------
SW = 10                      # open counters at small TFT heading sizes
T, M, B, W = 56, 80, 104, 34 # top / middle / bottom centerlines, letter width
TOP, BOT = 50, 110
GAP = 10                     # retain separation without oversized headings
WIDTH_SCALE = 0.86           # fit long titles beside the persistent status bar

UPM = 1000
CAP = 700                    # cap height in font units
S = CAP / (BOT - TOP)        # design -> font scale (11.667)


# ---- primitive builders (return lists of polygons, design coords, y down) --
def H(xa, xb, yc):
    return [[(xa, yc - SW / 2), (xb, yc - SW / 2), (xb, yc + SW / 2), (xa, yc + SW / 2)]]

def V(xc, ya, yb):
    return [[(xc - SW / 2, ya), (xc + SW / 2, ya), (xc + SW / 2, yb), (xc - SW / 2, yb)]]

def R(x1, y1, x2, y2):
    return [[(x1, y1), (x2, y1), (x2, y2), (x1, y2)]]

def clip_x(poly, xmin=None, xmax=None):
    """Sutherland-Hodgman clip against vertical half-planes."""
    def clip(pts, inside, inter):
        out = []
        for i, p in enumerate(pts):
            q = pts[(i + 1) % len(pts)]
            pi, qi = inside(p), inside(q)
            if pi:
                out.append(p)
            if pi != qi:
                out.append(inter(p, q))
        return out
    pts = list(poly)
    if xmin is not None:
        pts = clip(pts, lambda p: p[0] >= xmin,
                   lambda p, q: (xmin, p[1] + (q[1] - p[1]) * (xmin - p[0]) / (q[0] - p[0])))
    if xmax is not None and pts:
        pts = clip(pts, lambda p: p[0] <= xmax,
                   lambda p, q: (xmax, p[1] + (q[1] - p[1]) * (xmax - p[0]) / (q[0] - p[0])))
    return pts

def diag(x1, y1, x2, y2, xmin=None, xmax=None):
    """Diagonal stroke with horizontal cuts (flat on baseline / cap line)."""
    k = (x2 - x1) / (y2 - y1)
    t = SW * math.sqrt(1 + k * k)          # horizontal thickness
    xc = lambda y: x1 + k * (y - y1)
    poly = [(xc(y1) - t / 2, y1), (xc(y1) + t / 2, y1),
            (xc(y2) + t / 2, y2), (xc(y2) - t / 2, y2)]
    return [clip_x(poly, xmin, xmax)]


# ---- letters (local coords: x0 = 0 is the left stroke centerline) ---------
def letter_E():
    return V(0, TOP, BOT) + H(0, W, T) + H(0, W, B) + H(0, 28, M)

def letter_S():
    return (H(-6, W, T) + V(0, TOP, M + 6) + H(-6, W + 6, M)
            + V(W, M - 6, BOT) + H(-6, W, B))

def letter_C():
    return H(-6, W, T) + V(0, TOP, BOT) + H(0, W, B)

def letter_A():
    k = 13 / 50.0
    xl = lambda y: 13 - k * (y - 56)       # left leg centerline
    legs = diag(xl(TOP), TOP, xl(BOT), BOT) + diag(34 - xl(TOP), TOP, 34 - xl(BOT), BOT)
    return legs + H(5, 29, 88)

def letter_P():
    return V(0, TOP, BOT) + H(0, W, T) + V(W, TOP, M + 6) + H(0, W, M)

def letter_R():
    return (V(0, TOP, BOT) + H(0, W, T) + V(W, TOP, M + 6) + H(0, W, M)
            + diag(16, M, 34, BOT))

def letter_I():
    return V(0, TOP, BOT)

def letter_O(core=True):
    g = V(0, TOP, BOT) + V(W, TOP, BOT) + H(0, W, T) + H(0, W, B)
    if core:
        g += R(11, 74, 23, 86)
    return g

def letter_M():
    k = 22 / 26.0
    left = diag(0 - k * (56 - TOP), TOP, 22, 82, xmin=-6, xmax=50)
    right = diag(44 + k * (56 - TOP), TOP, 22, 82, xmin=-6, xmax=50)
    return V(0, TOP, BOT) + V(44, TOP, BOT) + left + right



def mirror(polys, cx):
    return [[(2 * cx - x, y) for x, y in p] for p in polys]

def poly(*pts):
    return [list(pts)]

# ---- additional letters ------------------------------------------------------
def letter_B():
    return (V(0, TOP, BOT) + H(0, W, T) + H(0, W, M) + H(0, W, B)
            + V(W, TOP, 76) + V(W, 84, BOT))

def letter_D():
    return V(0, TOP, BOT) + poly((0, 50), (32, 50), (40, 58), (40, 102), (32, 110),
                                 (0, 110), (0, 98), (28, 98), (28, 62), (0, 62))

def letter_F():
    return V(0, TOP, BOT) + H(0, W, T) + H(0, 28, M)

def letter_G():
    return H(-6, W, T) + V(0, TOP, BOT) + H(0, W, B) + V(W, M - 6, BOT) + H(18, W, M)

def letter_H():
    return V(0, TOP, BOT) + V(W, TOP, BOT) + H(0, W, M)

def letter_J():
    return V(W, TOP, BOT) + H(0, W, B) + V(0, M, BOT)

def letter_K():
    return (V(0, TOP, BOT) + diag(32, TOP, 4, 80, xmin=-6, xmax=40)
            + diag(4, 80, 32, BOT, xmin=-6, xmax=40))

def letter_L():
    return V(0, TOP, BOT) + H(0, W, B)

def letter_N():
    return V(0, TOP, BOT) + V(W, TOP, BOT) + diag(0, TOP, W, BOT, xmin=-6, xmax=40)

def letter_Q():
    return letter_O(core=False) + diag(22, 86, 34, BOT + 8, xmax=46)

def letter_T():
    return H(-6, W + 6, T) + V(17, TOP, BOT)

def letter_U():
    return V(0, TOP, BOT) + V(W, TOP, BOT) + H(0, W, B)

def letter_V():
    return (diag(0, TOP, 17, BOT, xmin=-6, xmax=40) + diag(W, TOP, 17, BOT, xmin=-6, xmax=40))

def letter_W():
    k = 22 / 26.0
    x = -k * (BOT - 104)
    return (V(0, TOP, BOT) + V(44, TOP, BOT)
            + diag(x, BOT, 22, 78, xmin=-6, xmax=50) + diag(44 - x, BOT, 22, 78, xmin=-6, xmax=50))

def letter_X():
    return diag(0, TOP, W, BOT, xmin=-6, xmax=40) + diag(W, TOP, 0, BOT, xmin=-6, xmax=40)

def letter_Y():
    return (diag(0, TOP, 17, 80, xmin=-6, xmax=40) + diag(W, TOP, 17, 80, xmin=-6, xmax=40)
            + V(17, 80, BOT))

def letter_Z():
    return H(-6, 40, T) + H(-6, 40, B) + diag(W, T, 0, B, xmin=-6, xmax=40)

# ---- digits ----------------------------------------------------------------
def d0(): return letter_O(core=False) + R(13, 68, 21, 92)
def d1(): return V(17, TOP, BOT) + H(2, 17, T) + H(0, W, B)
def d2(): return H(-6, W, T) + V(W, TOP, M + 6) + H(0, W, M) + V(0, M - 6, BOT) + H(0, 40, B)
def d3(): return V(W, TOP, BOT) + H(-6, W, T) + H(6, W, M) + H(-6, W, B)
def d4(): return V(0, TOP, 86) + V(W, TOP, BOT) + H(0, W, M)
def d5(): return H(-6, 40, T) + V(0, TOP, M + 6) + H(-6, W, M) + V(W, M - 6, BOT) + H(-6, W, B)
def d6(): return V(0, TOP, BOT) + H(0, W, T) + H(0, W, M) + H(0, W, B) + V(W, M - 6, BOT)
def d7(): return H(-6, 40, T) + diag(W, T, 12, BOT, xmin=-6, xmax=40)
def d8(): return V(0, TOP, BOT) + V(W, TOP, BOT) + H(0, W, T) + H(0, W, M) + H(0, W, B)
def d9(): return V(W, TOP, BOT) + H(0, W, T) + H(0, W, M) + H(0, W, B) + V(0, TOP, M + 6)

# ---- symbols ---------------------------------------------------------------
DOT = lambda: R(0, 98, 12, 110)
def sym_comma(): return DOT() + poly((0, 110), (12, 110), (4, 124), (-4, 124))
def sym_paren():
    return (R(-6, 62, 6, 98) + poly((-6, 62), (2, 50), (18, 50), (18, 62))
            + poly((-6, 98), (18, 98), (18, 110), (2, 110)))
def sym_bracket(): return V(0, TOP, BOT) + H(0, 16, T) + H(0, 16, B)

GLYPHS = {
    "A": letter_A, "B": letter_B, "C": letter_C, "D": letter_D, "E": letter_E,
    "F": letter_F, "G": letter_G, "H": letter_H, "I": letter_I, "J": letter_J,
    "K": letter_K, "L": letter_L, "M": letter_M, "N": letter_N, "O": letter_O,
    "P": letter_P, "Q": letter_Q, "R": letter_R, "S": letter_S, "T": letter_T,
    "U": letter_U, "V": letter_V, "W": letter_W, "X": letter_X, "Y": letter_Y,
    "Z": letter_Z,
    "0": d0, "1": d1, "2": d2, "3": d3, "4": d4, "5": d5, "6": d6, "7": d7, "8": d8, "9": d9,
    ".": DOT, ",": sym_comma,
    ":": lambda: R(0, 62, 12, 74) + DOT(),
    ";": lambda: R(0, 62, 12, 74) + sym_comma(),
    "!": lambda: V(6, TOP, 92) + DOT(),
    "?": lambda: H(-6, W, T) + V(W, TOP, M + 6) + H(11, W, M) + V(17, M - 6, 92) + R(11, 98, 23, 110),
    "-": lambda: H(0, 26, M),
    "_": lambda: R(0, 110, 40, 122),
    "+": lambda: H(0, W, M) + V(17, 63, 97),
    "=": lambda: H(0, W, 70) + H(0, W, 90),
    "/": lambda: diag(36, TOP, -2, BOT),
    "\\": lambda: diag(-2, TOP, 36, BOT),
    "(": sym_paren, ")": lambda: mirror(sym_paren(), 6),
    "[": sym_bracket, "]": lambda: mirror(sym_bracket(), 8),
    "#": lambda: V(6, TOP, BOT) + V(28, TOP, BOT) + H(-6, 40, 66) + H(-6, 40, 94),
    "%": lambda: R(-6, 50, 10, 66) + R(24, 94, 40, 110) + diag(34, 56, 6, 104),
    "$": lambda: letter_S() + V(17, 42, 118),
    '"': lambda: V(0, TOP, 70) + V(18, TOP, 70),
    "'": lambda: V(6, TOP, 70),
    "|": lambda: V(6, 42, 122),
    "O.plain": lambda: letter_O(core=False),
}


# ---- conversion to font glyphs --------------------------------------------
def area(pts):
    return sum(pts[i][0] * pts[(i + 1) % len(pts)][1] - pts[(i + 1) % len(pts)][0] * pts[i][1]
               for i in range(len(pts))) / 2

def build_glyph(polys):
    xs = [p[0] for poly in polys for p in poly]
    xmin, xmax = min(xs), max(xs)
    bearing = GAP / 2
    pen = TTGlyphPen(None)
    for poly in polys:
        pts = [(round((x - xmin + bearing) * S * WIDTH_SCALE), round((BOT - y) * S)) for x, y in poly]
        if area(pts) > 0:                   # TrueType outer contours: clockwise
            pts.reverse()
        pen.moveTo(pts[0])
        for p in pts[1:]:
            pen.lineTo(p)
        pen.closePath()
    advance = round((xmax - xmin + GAP) * S * WIDTH_SCALE)
    return pen.glyph(), advance


def accented(base, accent):
    polygons = GLYPHS[base]()
    xs = [x for polygon in polygons for x, _ in polygon]
    cx = (min(xs) + max(xs)) / 2
    if accent == '\u0300': mark = poly((cx - 8, 35), (cx - 2, 35), (cx + 6, 45), (cx, 45))
    elif accent == '\u0301': mark = poly((cx + 2, 35), (cx + 8, 35), (cx, 45), (cx - 6, 45))
    elif accent == '\u0302': mark = poly((cx - 10, 45), (cx, 34), (cx + 10, 45), (cx + 4, 45), (cx, 40), (cx - 4, 45))
    elif accent == '\u0308': mark = R(cx - 10, 36, cx - 3, 43) + R(cx + 3, 36, cx + 10, 43)
    elif accent == '\u0327': mark = poly((cx, 108), (cx + 6, 108), (cx + 4, 119), (cx - 6, 125), (cx - 10, 120), (cx - 2, 115))
    else: mark = poly((cx - 11, 43), (cx - 5, 35), (cx + 3, 39), (cx + 9, 34), (cx + 12, 39), (cx + 4, 46), (cx - 4, 42), (cx - 7, 46))
    return polygons + mark

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output-dir', type=Path, default=Path('assets/fonts'))
    args = parser.parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)
    # Latin-1 accents share the logo's uppercase forms, including lowercase aliases.
    for ch in 'ÀÁÂÄÈÉÊËÌÍÎÏÒÓÔÖÙÚÛÜÑÇ':
        base, accent = unicodedata.normalize('NFD', ch)
        GLYPHS[ch] = lambda base=base, accent=accent: accented(base, accent)
    GLYPHS['\u2018'] = GLYPHS["'"]
    GLYPHS['\u2019'] = GLYPHS["'"]
    GLYPHS['\u2013'] = GLYPHS['-']
    GLYPHS['\u2014'] = lambda: H(0, 40, M)
    GLYPHS['€'] = lambda: letter_C() + H(-9, 23, 73) + H(-9, 23, 87)
    gname = lambda ch: ch if ch == "O.plain" else "uni%04X" % ord(ch)
    order = [".notdef", "space"] + [gname(c) for c in GLYPHS]
    glyphs, metrics = {}, {}

    empty = TTGlyphPen(None).glyph()
    glyphs[".notdef"] = empty; metrics[".notdef"] = (500, 0)
    glyphs["space"] = empty;   metrics["space"] = (300, 0)

    for ch, fn in GLYPHS.items():
        name = gname(ch)
        g, adv = build_glyph(fn())
        glyphs[name] = g
        metrics[name] = (adv, round(GAP / 2 * S * WIDTH_SCALE))

    cmap = {0x20: "space"}
    for ch in GLYPHS:
        if ch == "O.plain":
            continue
        cmap[ord(ch)] = gname(ch)
        if ch.isalpha():
            cmap[ord(ch.lower())] = gname(ch)
    cmap[ord("o")] = "O.plain"
    cmap[0xA0] = 'space'

    fb = FontBuilder(UPM, isTTF=True)
    fb.setupGlyphOrder(order)
    fb.setupCharacterMap(cmap)
    fb.setupGlyf(glyphs)
    fb.setupHorizontalMetrics(metrics)
    fb.setupHorizontalHeader(ascent=920, descent=-180)
    fb.setupNameTable({"familyName": "Escaperiom Display", "styleName": "Regular",
                       "uniqueFontIdentifier": "EscaperiomDisplay-Regular",
                       "fullName": "Escaperiom Display Regular",
                       "psName": "EscaperiomDisplay-Regular", "version": "Version 1.100"})
    fb.setupOS2(sTypoAscender=920, sTypoDescender=-180, usWinAscent=920, usWinDescent=180,
                sCapHeight=CAP, sxHeight=CAP, achVendID="ESCP")
    fb.setupPost()
    fb.setupDummyDSIG() if hasattr(fb, "setupDummyDSIG") else None

    # merge overlapping contours into clean outlines (needs skia-pathops)
    try:
        from fontTools.ttLib.removeOverlaps import removeOverlaps
        removeOverlaps(fb.font)
        print("overlaps removed")
    except Exception as e:
        print("skip removeOverlaps (pip install skia-pathops):", e)

    fb.font.recalcTimestamp = False
    fb.font['head'].created = fb.font['head'].modified = 3802896000
    font_path = args.output_dir / 'EscaperiomDisplay.ttf'
    fb.save(font_path)
    print("wrote EscaperiomDisplay.ttf")

    try:
        from fontTools.ttLib import TTFont
        f = TTFont(font_path); f.flavor = "woff2"; f.save(args.output_dir / 'EscaperiomDisplay.woff2')
        print("wrote EscaperiomDisplay.woff2")
    except Exception as e:
        print("skip woff2 (pip install brotli):", e)


if __name__ == "__main__":
    main()
