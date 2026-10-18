"""Stage 4 boss, DUNE WYRM: a sandworm in armoured segments.

The head is the core; nine body segments and a tail follow its path
(motion_t's chain), each one a part to shoot off. Uses boss.MATS, in its
own colors (BOSS4): bone-pale shell, rust plates, a glowing maw.
"""

import math

from boss import MATS
from kit import Sprite

BOSS4 = [
    None, (20, 10, 8),
    (70, 50, 40), (110, 84, 64), (156, 128, 98), (204, 180, 146), (240, 226, 196),  # 2-6 shell
    (110, 40, 20), (170, 70, 34), (220, 120, 60),                                   # 7-9 rust
    (255, 110, 30), (255, 220, 120),                                                # 10-11 maw
    (30, 14, 10), (64, 34, 24),                                                     # 12-13 dark
    (255, 70, 50), (255, 240, 200),                                                 # 14-15 eyes
]
BOSS4_WRECK = [
    None, (10, 6, 6),
    (40, 30, 26), (60, 46, 40), (84, 66, 56), (110, 90, 76), (136, 116, 98),
    (60, 30, 20), (84, 46, 30), (110, 70, 44),
    (110, 50, 20), (150, 100, 60),
    (16, 10, 8), (34, 22, 18),
    (90, 40, 30), (120, 100, 80),
]


def head():
    """48x48 head, maw open toward the bottom of the screen."""
    s = Sprite(48, 48)
    s.poly([(24, 2), (40, 8), (46, 22), (42, 38), (34, 46), (14, 46), (6, 38), (2, 22), (8, 8)], "armor", amp=3,
           bevel=6)
    s.poly([(24, 5), (35, 10), (39, 20), (24, 24), (9, 20), (13, 10)], "gold", z=2, amp=1.4, bevel=2.5)
    # The maw: a ring of teeth around a burning throat.
    s.ellipse((12, 22, 36, 46), "vent", z=2.5, amp=0.4, profile="flat")
    s.ellipse((16, 26, 32, 42), "core", z=3)
    s.ellipse((20, 30, 28, 38), "corehot", z=4)
    for k in range(8):
        a = math.radians(k * 45 + 22)
        x, y = 24 + math.cos(a) * 11, 34 + math.sin(a) * 11
        s.poly([(x, y), (24 + math.cos(a) * 7, 34 + math.sin(a) * 7 - 1.5),
                (24 + math.cos(a) * 7 + 1.5, 34 + math.sin(a) * 7)], "plate", z=4.5, amp=0.8, bevel=0.8)
    s.ellipse((9, 12, 15, 18), "light", z=4, sym=True)
    s.dot(11, 14, "lightb", z=5, sym=True)
    return [s.render(MATS, gloss=0.4)]


def segment():
    """32x32 body ring: a shell plate over a rusted band. Frames: two, so
    neighbors can differ."""
    out = []
    for k in range(2):
        s = Sprite(32, 32)
        s.ellipse((1, 3, 31, 29), "armor", amp=2.6)
        s.ellipse((5, 6, 27, 26), "gold" if k else "plate", z=2, amp=1.2)
        for x in (8, 16, 24):
            s.poly([(x - 2, 6), (x, 1), (x + 2, 6)], "plate", z=3, amp=1, bevel=1)
        s.rect((6, 15, 26, 17), "vent", z=2.6, amp=0.3, profile="flat")
        out.append(s.render(MATS, gloss=0.35))
    return out


def tail():
    """32x32 tail: tapering shell and a stinger."""
    s = Sprite(32, 32)
    s.poly([(6, 4), (26, 4), (22, 18), (16, 31), (10, 18)], "armor", amp=2.2, bevel=4)
    s.poly([(10, 6), (22, 6), (19, 15), (13, 15)], "gold", z=2, amp=1, bevel=1.5)
    s.poly([(14, 20), (16, 31), (18, 20)], "corehot", z=3)
    return [s.render(MATS, gloss=0.35)]
