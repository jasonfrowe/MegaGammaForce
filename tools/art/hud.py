"""HUD sprites, drawn on plane 2 in front of the text panels.

  mini_ship()     16x16 ship for the lives count (palette PLAYER)
  gems()          8x8 chain gems, unlit and lit (an enemy palette colours them)
  weapon_icons()  16x16 icon per weapon id, 0 to 9 (palette SHOTS)
"""

import math

from kit import Material, Shape, Sprite
from ships import PLAYER_MATS, player_sprite


def mini_ship():
    """The player ship at two thirds size."""
    big = player_sprite()
    s = Sprite(16, 16)
    k = 16 / 24
    for sh in big.shapes:
        s.shapes.append(Shape(sh.kind, [(x * k, y * k) for x, y in sh.pts], sh.mat, sh.z * k, sh.amp * k,
                              max(1.0, sh.bevel * k), max(1.0, sh.width * k), sh.profile))
    return [s.render(PLAYER_MATS, gloss=0.4)]


GEM_MATS = {
    "paint": Material([6, 7, 8, 9]),
    "dark": Material([2, 3]),
    "white": Material([15], shaded=False),
}


def gems():
    """A cut gem: frame 0 unlit (dark facets), frame 1 lit with a glint."""
    out = []
    for lit in (False, True):
        s = Sprite(8, 8)
        s.poly([(4, 0), (8, 4), (4, 8), (0, 4)], "paint" if lit else "dark", amp=1.6, bevel=2.5)
        if lit:
            s.dot(3, 2, "white", z=3)
        out.append(s.render(GEM_MATS, gloss=0.6))
    return out


ICON_MATS = {
    "cyan": Material([1, 2, 3, 4]),
    "white": Material([5], shaded=False),
    "orange": Material([6, 7, 8, 9]),
    "green": Material([10, 11, 12, 13]),
    "violet": Material([14, 15], dither=0.0),
}


def _icon(draw):
    s = Sprite(16, 16)
    draw(s)
    return s.render(ICON_MATS, gloss=0.5)


def weapon_icons():
    def none(s):
        pass

    def vulcan(s):
        for x in (4, 9):
            s.rect((x, 2, x + 3, 14), "cyan", amp=1.5, bevel=1.5)
            s.rect((x + 1, 2, x + 2, 6), "white", z=2)

    def homing(s):
        pts = [(3 + 10 * t, 14 - 12 * t + 6 * math.sin(t * math.pi)) for t in [i / 6 for i in range(7)]]
        for a, b in zip(pts, pts[1:]):
            s.line(a, b, "green", width=2)
        s.poly([(13, 1), (15, 6), (10, 4)], "green", z=1, amp=1.2, bevel=1)
        s.dot(13, 3, "white", z=2)

    def plasma(s):
        s.line((8, 15), (5, 10), "violet", width=2)
        s.line((5, 10), (11, 7), "violet", width=2)
        s.line((11, 7), (7, 1), "violet", width=2)
        s.line((8, 15), (5, 10), "white", width=1, z=1)
        s.line((11, 7), (7, 1), "white", width=1, z=1)
        s.ellipse((5, 0, 10, 4), "cyan", z=1)

    def spread(s):
        for a in (-64, -32, 0, 32, 64):
            r = math.radians(a - 90)
            x, y = 8 + math.cos(r) * 7, 14 + math.sin(r) * 11
            s.ellipse((x - 1.6, y - 1.6, x + 1.6, y + 1.6), "orange", z=1)
        s.ellipse((6.5, 12.5, 9.5, 15.5), "white", z=1)

    def wide(s):
        s.rect((6.5, 0, 9.5, 9), "cyan", amp=1.5, bevel=1.5)
        s.rect((7.5, 0, 8.5, 3), "white", z=2)
        for x0, x1 in ((7, 1), (9, 15)):
            s.line((x0, 10), (x1, 15), "orange", width=2)
        s.line((5, 11), (0, 11), "orange", width=2)
        s.line((11, 11), (16, 11), "orange", width=2)

    def missile(s):
        s.ellipse((0, 0, 16, 16), "green", amp=0.3, profile="flat")
        s.ellipse((2, 2, 14, 14), "violet", amp=0.3, profile="flat")
        s.ellipse((2.5, 2.5, 13.5, 13.5), "green", z=0.5, amp=0.3, profile="flat")
        s.ellipse((3.5, 3.5, 12.5, 12.5), "violet", z=0.6, amp=0.3, profile="flat")
        s.poly([(8, 1), (10, 5), (10, 12), (8, 14), (6, 12), (6, 5)], "orange", z=2, amp=1.5, bevel=1.5)
        s.dot(7, 4, "white", z=3)

    def sword(s):
        s.poly([(8, 0), (10.5, 3), (10, 11), (6, 11), (5.5, 3)], "cyan", amp=1.6, bevel=2)
        s.line((8, 1), (8, 10), "white", width=1, z=2)
        s.rect((3, 11, 13, 13), "orange", z=1, amp=1, bevel=1)
        s.rect((7, 13, 9, 16), "orange", z=1, amp=1, bevel=1)

    def hyper(s):
        s.ellipse((0, 0, 16, 16), "violet", amp=0.5, profile="flat")
        sword(s)

    order = [none, vulcan, homing, plasma, spread, wide, missile, vulcan, sword, hyper]
    return [_icon(f) for f in order]


# Score digits, hand drawn 6x12 with 2 pixel strokes, set in 8x16 cells.
DIGIT_ART = [
    [".####.", "##..##", "##..##", "##..##", "##..##", "##..##", "##..##", "##..##", "##..##", "##..##", "##..##",
     ".####."],
    ["..##..", ".###..", "####..", "..##..", "..##..", "..##..", "..##..", "..##..", "..##..", "..##..", "..##..",
     "######"],
    [".####.", "##..##", "....##", "....##", "...###", "..###.", ".###..", "###...", "##....", "##....", "##....",
     "######"],
    [".####.", "##..##", "....##", "....##", "....##", "..###.", "..###.", "....##", "....##", "....##", "##..##",
     ".####."],
    ["##..##", "##..##", "##..##", "##..##", "##..##", "######", "######", "....##", "....##", "....##", "....##",
     "....##"],
    ["######", "##....", "##....", "##....", "#####.", "....##", "....##", "....##", "....##", "....##", "##..##",
     ".####."],
    [".####.", "##..##", "##....", "##....", "#####.", "##..##", "##..##", "##..##", "##..##", "##..##", "##..##",
     ".####."],
    ["######", "....##", "....##", "...##.", "...##.", "..##..", "..##..", "..##..", "..##..", "..##..", "..##..",
     "..##.."],
    [".####.", "##..##", "##..##", "##..##", "##..##", ".####.", "##..##", "##..##", "##..##", "##..##", "##..##",
     ".####."],
    [".####.", "##..##", "##..##", "##..##", "##..##", "##..##", ".#####", "....##", "....##", "....##", "##..##",
     ".####."],
]


def digits(gold):
    """0-9 in 8x16 cells, shaded light to dark down the stroke, outlined:
    cyan for the score, gold for the high score (palette SHOTS)."""
    import numpy as np
    ramp = [9, 9, 9, 8, 8, 8, 8, 8, 7, 7, 7, 7] if gold else [4, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2]
    rim = 6 if gold else 1
    out = []
    for art in DIGIT_ART:
        f = np.zeros((16, 8), dtype=np.uint8)
        for y, row in enumerate(art):
            for x, c in enumerate(row):
                if c == "#":
                    f[y + 2, x + 1] = ramp[y]
        lit = f != 0
        # A glint on the top-left of each stroke.
        for y in range(16):
            for x in range(8):
                if lit[y, x] and not lit[y - 1, x] and not lit[y, x - 1]:
                    f[y, x] = 5
        pad = np.pad(lit, 1)
        nb = pad[:-2, 1:-1] | pad[2:, 1:-1] | pad[1:-1, :-2] | pad[1:-1, 2:]
        f[(~lit) & nb] = rim
        out.append(f)
    return out


# 5x7 capitals for the stage banner.
GLYPHS = {
    "S": [".XXXX", "X....", "X....", ".XXX.", "....X", "....X", "XXXX."],
    "T": ["XXXXX", "..X..", "..X..", "..X..", "..X..", "..X..", "..X.."],
    "A": [".XXX.", "X...X", "X...X", "XXXXX", "X...X", "X...X", "X...X"],
    "G": [".XXXX", "X....", "X....", "X.XXX", "X...X", "X...X", ".XXXX"],
    "E": ["XXXXX", "X....", "X....", "XXXX.", "X....", "X....", "XXXXX"],
    "W": ["X...X", "X...X", "X...X", "X.X.X", "X.X.X", "XX.XX", "X...X"],
    "R": ["XXXX.", "X...X", "X...X", "XXXX.", "X.X..", "X..X.", "X...X"],
    "N": ["X...X", "XX..X", "X.X.X", "X..XX", "X...X", "X...X", "X...X"],
    "I": [".XXX.", "..X..", "..X..", "..X..", "..X..", "..X..", ".XXX."],
    "1": ["..X..", ".XX..", "..X..", "..X..", "..X..", "..X..", ".XXX."],
    "2": [".XXX.", "X...X", "....X", "...X.", "..X..", ".X...", "XXXXX"],
    "3": ["XXXX.", "....X", "....X", ".XXX.", "....X", "....X", "XXXX."],
    "4": ["X..X.", "X..X.", "X..X.", "XXXXX", "...X.", "...X.", "...X."],
    "5": ["XXXXX", "X....", "XXXX.", "....X", "....X", "X...X", ".XXX."],
    "6": [".XXX.", "X....", "X....", "XXXX.", "X...X", "X...X", ".XXX."],
    "7": ["XXXXX", "....X", "...X.", "..X..", "..X..", ".X...", ".X..."],
    "8": [".XXX.", "X...X", "X...X", ".XXX.", "X...X", "X...X", ".XXX."],
    "9": [".XXX.", "X...X", "X...X", ".XXXX", "....X", "....X", ".XXX."],
}

BANNER_MATS = {"gold": Material([7, 8, 9, 5]), "edge": Material([6], shaded=False)}


def _glyphs(s, text, x0, y0, cell=2):
    for k, ch in enumerate(text):
        g = GLYPHS[ch]
        for gy, row in enumerate(g):
            for gx, c in enumerate(row):
                if c == "X":
                    x, y = x0 + k * 12 + gx * cell, y0 + gy * cell
                    s.rect((x, y, x + cell, y + cell), "gold", amp=1.0, bevel=1.0)


def banner():
    """The stage banner: "STAGE" in 64x16, then the digits 1-9 in 16x16 (palette SHOTS),
    drawn doubled."""
    word = Sprite(64, 16)
    _glyphs(word, "STAGE", 2, 1)
    out = [word.render(BANNER_MATS, gloss=0.4, outline=6)]
    nums = []
    for d in "123456789":
        s = Sprite(16, 16)
        _glyphs(s, d, 3, 1)
        nums.append(s.render(BANNER_MATS, gloss=0.4, outline=6))
    return out, nums


def portrait(img, size=16):
    """A boss's portrait for the HUD: its sprite shrunk to size x size,
    taking the commonest colour in each block, then outlined."""
    import numpy as np
    h, w = img.shape
    k = max(h, w) // size
    out = np.zeros((size, size), dtype=np.uint8)
    oy, ox = (size - h // k) // 2, (size - w // k) // 2
    for y in range(h // k):
        for x in range(w // k):
            block = img[y * k:(y + 1) * k, x * k:(x + 1) * k].ravel()
            block = block[block != 0]
            if len(block) * 2 >= k * k:
                vals, counts = np.unique(block, return_counts=True)
                out[oy + y, ox + x] = vals[np.argmax(counts)]
    filled = out != 0
    pad = np.pad(filled, 1)
    nb = pad[:-2, 1:-1] | pad[2:, 1:-1] | pad[1:-1, :-2] | pad[1:-1, 2:]
    out[(~filled) & nb] = 1
    return [out]


def warning():
    """"WARNING", 88x16, cut into a 64 and a 24 wide piece (sprites are at
    most 64 wide), drawn doubled. The same art flashes gold with palette
    SHOTS and red with ENEMY_RED, whose entries 5-9 line up with it."""
    s = Sprite(88, 16)
    _glyphs(s, "WARNING", 2, 1)
    img = s.render(BANNER_MATS, gloss=0.4, outline=6)
    return [img[:, :64].copy()], [img[:, 64:].copy()]
