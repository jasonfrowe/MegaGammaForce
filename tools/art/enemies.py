"""Enemy sprites. Paint (palette 6-9) carries the chain color, swapped by palette."""

import math

import numpy as np

from kit import Material, Sprite

MATS = {
    "metal": Material([2, 3, 4, 5]),
    "paint": Material([6, 7, 8, 9]),
    "glow": Material([10, 11], shaded=False),
    "glowhot": Material([11], shaded=False),
    "canopy": Material([12, 13], dither=0.0),
    "dark": Material([14], shaded=False),
    "white": Material([15], shaded=False),
}


def _with_glow(s: Sprite, mats, hot: bool):
    m = dict(mats)
    m["glow"] = Material([11] if hot else [10], shaded=False)
    return s.render(m, gloss=0.3)


def dart():
    """16x16 swept fighter, nose down. Frames: engine dim, engine hot."""
    s = Sprite(16, 16)
    s.poly([(8, 15), (6, 8), (1, 2), (3, 1), (8, 5)], "paint", z=0, amp=1.2, bevel=1.5, sym=True)
    s.poly([(8, 16), (6.5, 9), (6.5, 1), (9.5, 1), (9.5, 9)], "metal", z=1, amp=2, bevel=2)
    s.ellipse((6.5, 7, 9.5, 12), "canopy", z=2.5, amp=1)
    s.rect((6.5, 0, 9.5, 2), "glow", z=1)
    return [_with_glow(s, MATS, h) for h in (False, True)]


def saucer():
    """16x16 disc with a ring of running lights. 4 spin frames."""
    frames = []
    for f in range(4):
        s = Sprite(16, 16)
        s.ellipse((0, 2, 16, 14), "paint", z=0, amp=1.0)
        s.ellipse((3, 4, 13, 12), "metal", z=1.0, amp=1.4)
        s.ellipse((5.5, 5.5, 10.5, 10.5), "canopy", z=2.2, amp=1.2)
        for k in range(6):
            a = (k / 6.0 + f / 24.0) * 2 * math.pi
            x = 8 + math.cos(a) * 6.6
            y = 8 + math.sin(a) * 4.8
            s.dot(int(x), int(y), "glowhot" if k % 3 == f % 3 else "dark", z=1.5)
        frames.append(s.render(MATS, gloss=0.5))
    return frames


def drone():
    """16x16 armoured pod with a single eye. Frames: eye open, eye bright."""
    s = Sprite(16, 16)
    s.poly([(4, 1), (12, 1), (15, 6), (15, 10), (12, 15), (4, 15), (1, 10), (1, 6)], "metal", amp=1.5, bevel=2.5)
    s.poly([(5, 3), (11, 3), (13, 7), (13, 9), (11, 13), (5, 13), (3, 9), (3, 7)], "paint", z=1, amp=1.2, bevel=2)
    s.rect((0, 6, 2, 10), "dark", z=0.5, sym=True)
    s.ellipse((5, 5, 11, 11), "dark", z=1.5, amp=0.2, profile="flat")
    s.ellipse((6, 6, 10, 10), "glow", z=2)
    out = [_with_glow(s, MATS, h) for h in (False, True)]
    return out


def gunship():
    """32x32 heavy gunship, twin cannons, nose down. Frames: engines dim/hot."""
    s = Sprite(32, 32)
    # Wings
    s.poly([(16, 22), (3, 14), (1, 6), (5, 4), (16, 10)], "paint", z=0, amp=1.4, bevel=2.5, sym=True)
    s.poly([(5, 6), (3, 13), (8, 16), (8, 6)], "metal", z=0.6, amp=1, bevel=1.5, sym=True)
    # Cannon pods
    s.rect((6, 10, 10, 26), "metal", z=1, amp=1.6, bevel=1.5, sym=True)
    s.rect((7, 25, 9, 29), "dark", z=1, sym=True)
    # Hull
    s.poly([(16, 31), (12, 22), (11, 6), (13, 1), (19, 1), (21, 6), (20, 22)], "metal", z=1.5, amp=3, bevel=3)
    s.poly([(16, 28), (13.5, 21), (13.5, 12), (18.5, 12), (18.5, 21)], "paint", z=3, amp=1, bevel=1.5)
    s.ellipse((13.5, 16, 18.5, 24), "canopy", z=4, amp=1.2)
    # Engines at the back (top of the sprite)
    s.rect((12, 0, 15, 3), "glow", z=2)
    s.rect((17, 0, 20, 3), "glow", z=2)
    s.dot(4, 7, "glowhot", z=1, sym=True)
    return [_with_glow(s, MATS, h) for h in (False, True)]


def turret_frames():
    """16x16 ground turret, barrel aimed in 8 directions starting straight down, clockwise."""
    frames = []
    for k in range(8):
        a = math.pi / 2 + k * (2 * math.pi / 8)  # 0 = down (screen +y)
        s = Sprite(16, 16)
        s.poly([(3, 1), (13, 1), (15, 3), (15, 13), (13, 15), (3, 15), (1, 13), (1, 3)], "metal", amp=1.2, bevel=2)
        s.ellipse((3, 3, 13, 13), "paint", z=1, amp=1.5)
        ex, ey = 8 + math.cos(a) * 7.5, 8 + math.sin(a) * 7.5
        s.line((8, 8), (ex, ey), "dark", width=3, z=3)
        s.line((8, 8), (ex, ey), "metal", width=1, z=3.5)
        s.ellipse((5.5, 5.5, 10.5, 10.5), "metal", z=3, amp=1.2)
        s.dot(7, 7, "glow", z=4)
        frames.append(s.render(MATS, gloss=0.3))
    return frames
