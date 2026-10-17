"""Stage 2 boss, SPIRE WARDEN: a hovering defence core with three pods
that orbit it, a heavy cannon slung under it and two shoulder turrets.

Uses the same palette layout as the stage 1 boss (boss.MATS), with the
stage's own colors (palettes.BOSS2): violet armour, teal trim, a magenta eye.
"""

import math

from boss import MATS
from kit import Sprite


def core():
    """64x64 octagonal hull with a slit eye, nose down."""
    s = Sprite(64, 64)
    s.poly([(20, 2), (44, 2), (60, 18), (60, 42), (44, 60), (20, 60), (4, 42), (4, 18)], "armor", amp=3, bevel=6)
    s.poly([(24, 8), (40, 8), (52, 20), (52, 40), (40, 52), (24, 52), (12, 40), (12, 20)], "plate", z=2, amp=1.4,
           bevel=3)
    # Teal ribs from the eye to the rim.
    for a in range(0, 360, 45):
        x0, y0 = 32 + math.cos(math.radians(a)) * 13, 30 + math.sin(math.radians(a)) * 13
        x1, y1 = 32 + math.cos(math.radians(a)) * 26, 30 + math.sin(math.radians(a)) * 26
        s.line((x0, y0), (x1, y1), "gold", width=2, z=3)
    s.ellipse((19, 17, 45, 43), "vent", z=3, amp=0.4, profile="flat")
    # The eye: a vertical slit in a glowing iris.
    s.ellipse((22, 20, 42, 40), "core", z=4)
    s.ellipse((28, 21, 36, 39), "corehot", z=5)
    s.rect((31, 22, 33, 38), "vent", z=6, amp=0.2, profile="flat")
    # Spire on top, and the cannon mount below.
    s.poly([(28, 2), (32, 0), (36, 2), (35, 10), (29, 10)], "gold", z=3, amp=1.2, bevel=1.5)
    s.rect((26, 52, 38, 62), "plate", z=2.5, amp=1.2, bevel=1.5)
    for y in (24, 36):
        s.dot(7, y, "light", z=3, sym=True)
    s.dot(32, 4, "lightb", z=4)
    return [s.render(MATS, gloss=0.35)]


def pod():
    """32x32 orbiting pod: an armoured ball with four fins. Frames: glow dim/bright."""
    out = []
    for hot in (False, True):
        s = Sprite(32, 32)
        for a in (45, 135, 225, 315):
            c, sn = math.cos(math.radians(a)), math.sin(math.radians(a))
            tip = (16 + c * 15.5, 16 + sn * 15.5)
            l = (16 + c * 8 - sn * 4, 16 + sn * 8 + c * 4)
            r = (16 + c * 8 + sn * 4, 16 + sn * 8 - c * 4)
            s.poly([tip, l, r], "gold", z=0.5, amp=1.2, bevel=1.5)
        s.ellipse((5, 5, 27, 27), "armor", z=1, amp=2.2)
        s.ellipse((9, 9, 23, 23), "plate", z=2.5, amp=1.2)
        s.ellipse((11.5, 11.5, 20.5, 20.5), "corehot" if hot else "core", z=4)
        s.ellipse((14, 14, 18, 18), "lightb" if hot else "core", z=5)
        out.append(s.render(MATS, gloss=0.4))
    return out


def cannon():
    """32x32 twin cannon under the core, barrels down. Frames: idle, charging."""
    out = []
    for hot in (False, True):
        s = Sprite(32, 32)
        s.poly([(4, 2), (28, 2), (30, 12), (24, 20), (8, 20), (2, 12)], "armor", amp=2, bevel=4)
        for x in (9, 19):
            s.rect((x, 14, x + 4, 31), "plate", z=2, amp=1.4, bevel=1.5)
            s.rect((x + 1, 27, x + 3, 32), "core" if hot else "vent", z=3)
        s.ellipse((11, 4, 21, 14), "gold", z=2.5, amp=1.2)
        s.ellipse((13, 6, 19, 12), "corehot" if hot else "core", z=3.5)
        s.dot(4, 8, "light", z=3, sym=True)
        out.append(s.render(MATS, gloss=0.3))
    return out
