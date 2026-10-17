"""Stage 1 boss, GAMMA CORE: a fortress-ship assembled from separate parts.

Each part is its own sprite with its own hit points, so it can be shot
off. Destroyed parts are redrawn with the BOSS_WRECK palette.
"""

import math

from kit import Material, Sprite

MATS = {
    "armor": Material([2, 3, 4, 5, 6]),
    "plate": Material([2, 3, 4, 5]),
    "gold": Material([7, 8, 9]),
    "core": Material([10], shaded=False),
    "corehot": Material([11], shaded=False),
    "vent": Material([12, 13], dither=0.0),
    "light": Material([14], shaded=False),
    "lightb": Material([15], shaded=False),
}


def core():
    """64x64 central hull, nose down. The glowing core eye is the weak point."""
    s = Sprite(64, 64)
    # Outer hull: wide armoured shield.
    s.poly([(32, 63), (18, 54), (8, 36), (8, 14), (16, 3), (48, 3), (56, 14), (56, 36), (46, 54)],
           "armor", amp=3, bevel=6)
    # Shoulder plates.
    s.poly([(9, 15), (17, 5), (25, 5), (22, 18), (12, 30)], "plate", z=2, amp=1.6, bevel=2.5, sym=True)
    # Gold collar around the core.
    s.ellipse((17, 19, 47, 49), "gold", z=3, amp=1.6)
    s.ellipse((20, 22, 44, 46), "vent", z=3.5, amp=0.4, profile="flat")
    # Core eye.
    s.ellipse((23, 25, 41, 43), "core", z=5)
    s.ellipse((27, 29, 37, 39), "corehot", z=6)
    s.ellipse((29, 30, 33, 34), "lightb", z=7)
    # Bow ram.
    s.poly([(32, 63), (27, 50), (32, 47), (37, 50)], "plate", z=3, amp=1.5, bevel=2)
    # Vents along the flanks.
    for y in (36, 40, 44):
        s.rect((11, y, 16, y + 2), "vent", z=2.2, sym=True)
    # Engine bays at the stern.
    s.rect((22, 3, 28, 9), "vent", z=2, sym=True)
    s.rect((23, 0, 27, 4), "light", z=2.5, sym=True)
    # Running lights.
    for y in (20, 28):
        s.dot(10, y, "lightb", z=3, sym=True)
    s.line((32, 10), (32, 18), "light", z=4)
    return [s.render(MATS, gloss=0.35)]


def wing():
    """32x64 left wing. The right wing is drawn with MODE5_HFLIP."""
    s = Sprite(32, 64)
    s.poly([(31, 6), (31, 54), (20, 60), (6, 50), (1, 30), (4, 12), (14, 2)], "armor", amp=2.4, bevel=5)
    s.poly([(28, 10), (28, 48), (18, 54), (9, 46), (6, 30), (8, 16), (16, 8)], "plate", z=2, amp=1.2, bevel=2.5)
    # Turret sockets (turret sprites sit on top of these).
    s.ellipse((9, 13, 25, 29), "vent", z=2.5, amp=0.3, profile="flat")
    s.ellipse((9, 35, 25, 51), "vent", z=2.5, amp=0.3, profile="flat")
    # Gold edge strip.
    s.poly([(1, 30), (4, 12), (6, 13), (3, 30), (6, 49), (4, 50)], "gold", z=2.5, amp=0.8, bevel=1)
    for y in (8, 32, 56):
        s.dot(29, y, "light", z=3)
    s.dot(5, 31, "lightb", z=3)
    return [s.render(MATS, gloss=0.3)]


def cannon():
    """32x32 heavy cannon pod hung under the core. Frames: idle, charging."""
    out = []
    for hot in (False, True):
        s = Sprite(32, 32)
        s.poly([(6, 2), (26, 2), (30, 10), (28, 22), (4, 22), (2, 10)], "armor", amp=2, bevel=4)
        s.rect((12, 18, 20, 31), "plate", z=2, amp=1.5, bevel=2)
        s.rect((13.5, 27, 18.5, 32), "vent", z=3)
        s.ellipse((10, 6, 22, 18), "gold", z=2.5, amp=1.2)
        s.ellipse((12.5, 8.5, 19.5, 15.5), "corehot" if hot else "core", z=3.5)
        s.dot(15, 29, "corehot" if hot else "vent", z=4)
        s.dot(4, 10, "light", z=3, sym=True)
        out.append(s.render(MATS, gloss=0.3))
    return out


def turret():
    """16x16 boss turret, barrel in 8 directions starting straight down, clockwise."""
    out = []
    for k in range(8):
        a = math.pi / 2 + k * (2 * math.pi / 8)
        s = Sprite(16, 16)
        s.ellipse((1, 1, 15, 15), "armor", amp=1.5)
        s.ellipse((4, 4, 12, 12), "gold", z=1.5, amp=1.0)
        ex, ey = 8 + math.cos(a) * 7.8, 8 + math.sin(a) * 7.8
        s.line((8, 8), (ex, ey), "vent", width=3, z=3)
        s.line((8, 8), (ex, ey), "plate", width=1, z=3.5)
        s.ellipse((6, 6, 10, 10), "core", z=4)
        out.append(s.render(MATS, gloss=0.3))
    return out
