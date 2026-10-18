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
