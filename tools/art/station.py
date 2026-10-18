"""Stage 6 boss, HALO STATION: an orbital ring station.

The station is a tile map (8 x 8 tiles, 128 x 128 pixels) on plane 1's
layer, palette STATION: a ring on four spokes around a hub. Running
lights chase around the ring in the rotating entries 2-4. Six turrets sit
on the ring and the core in the hub (sprites, palette BOSS6); four
satellites orbit outside it. Destroyed parts leave craters.
"""

import math

import numpy as np

from boss import MATS
from kit import Material, Sprite, value_noise

T = 16
W, H = 8, 8

STATION = [
    None,                                                               # 0 clear
    (6, 8, 18),                                                         # 1 outline
    (40, 120, 200), (90, 200, 255), (210, 250, 255),                    # 2-4 running lights (rotate)
    (46, 50, 64), (78, 84, 102), (120, 128, 148), (170, 178, 196), (226, 230, 240),  # 5-9 hull
    (14, 16, 26),                                                       # 10 shadow
    (214, 164, 44),                                                     # 11 gold foil
    (30, 64, 150),                                                      # 12 solar blue
    (255, 50, 50), (80, 255, 140),                                      # 13-14 lamps (blink)
    (255, 236, 160),                                                    # 15 windows
]
OUT, SH, FOIL, SOLAR, LAMP_R, LAMP_G, WIN = 1, 10, 11, 12, 13, 14, 15
HULL = [5, 6, 7, 8, 9]
RUN = [2, 3, 4]

BOSS6 = [
    None, (6, 8, 18),
    (46, 50, 64), (78, 84, 102), (120, 128, 148), (170, 178, 196), (226, 230, 240),  # 2-6 armour
    (130, 90, 20), (214, 164, 44), (255, 226, 130),                                # 7-9 gold foil
    (255, 70, 200), (255, 200, 245),                                               # 10-11 core
    (14, 16, 26), (36, 40, 60),                                                    # 12-13 vents
    (60, 170, 255), (220, 250, 255),                                               # 14-15 lights
]
BOSS6_WRECK = [
    None, (6, 6, 8),
    (26, 24, 28), (40, 38, 42), (58, 56, 60), (80, 76, 78), (104, 98, 98),
    (50, 36, 20), (76, 56, 30), (100, 80, 50),
    (100, 40, 70), (150, 100, 120),
    (10, 10, 12), (24, 22, 26),
    (40, 60, 80), (80, 90, 100),
]

# Where each part sits: tile column and row of its top left, size in tiles.
PARTS = {
    "CORE": (3, 3, 2, 2),
    "TUR0": (0, 3, 1, 1),
    "TUR1": (7, 4, 1, 1),
    "TUR2": (1, 1, 1, 1),
    "TUR3": (6, 1, 1, 1),
    "TUR4": (1, 6, 1, 1),
    "TUR5": (6, 6, 1, 1),
}

HULL_MATS = {
    "hull": Material(HULL),
    "hub": Material(HULL[1:]),
    "foil": Material([FOIL], shaded=False),
    "dark": Material([SH, HULL[0]], dither=0.0),
}


def station_image():
    n = W * T
    c = n / 2 - 0.5
    yy, xx = np.mgrid[0:n, 0:n]
    d = np.sqrt((xx - c) ** 2 + (yy - c) ** 2)
    s = Sprite(n, n)
    ring = (d < 62) & (d > 46)
    s.mask(ring, "hull", amp=3, bevel=5)
    for a in (45, 135, 225, 315):
        r = math.radians(a)
        s.line((c + math.cos(r) * 16, c + math.sin(r) * 16), (c + math.cos(r) * 48, c + math.sin(r) * 48), "hull",
               width=5, z=0.5, amp=1.2)
    s.ellipse((c - 24, c - 24, c + 25, c + 25), "hub", z=1, amp=2.5)
    s.ellipse((c - 18, c - 18, c + 19, c + 19), "foil", z=3)
    img = s.render(HULL_MATS, gloss=0.25)
    ang = (np.degrees(np.arctan2(yy - c, xx - c)) + 360) % 360
    # Windows along the ring, running lights around its rim.
    win = ring & (np.abs(d - 54) < 1) & ((ang * 54 * math.pi / 180).astype(int) % 5 == 0)
    img[win] = WIN
    rim = (np.abs(d - 59.5) < 0.8) & ((ang * 60 * math.pi / 180).astype(int) % 4 == 0)
    img[rim] = np.array(RUN)[((ang[rim] // 6).astype(int)) % 3]
    # Solar strips across the hub, lamps at the spoke roots.
    for a in (45, 135, 225, 315):
        r = math.radians(a)
        x, y = int(c + math.cos(r) * 22), int(c + math.sin(r) * 22)
        img[y:y + 2, x:x + 2] = LAMP_R if a in (45, 225) else LAMP_G
    return img


def _socket(size, base):
    """A gun ring, size tiles across, drawn over base (that many tiles)."""
    n = size * T
    big = base.copy()
    yy, xx = np.mgrid[0:n, 0:n]
    c = n / 2 - 0.5
    d = np.sqrt((xx - c) ** 2 + (yy - c) ** 2)
    r = n / 2 - 1.5
    big[d < r] = HULL[1]
    big[(d < r) & (d > r - 1.5) & (xx + yy < 2 * c)] = HULL[4]
    big[(d < r) & (d > r - 1.5) & (xx + yy >= 2 * c)] = OUT
    big[d < r - 3] = SH
    return big


def _plate(seed, n=T):
    v = value_noise(n, n, 8, 2600 + seed)
    t = np.full((n, n), HULL[2], dtype=np.uint8)
    t[v > 0.7] = HULL[3]
    t[v < 0.28] = HULL[1]
    return t


def _crater(size, seed):
    n = size * T
    big = _plate(seed, n)
    yy, xx = np.mgrid[0:n, 0:n]
    c = n / 2 - 0.5
    noise = value_noise(n, n, 3, 2700 + seed)
    d = np.sqrt((xx - c) ** 2 + (yy - c) ** 2) / (n / 2) + (noise - 0.5) * 0.5
    big[d < 0.95] = HULL[0]
    big[d < 0.75] = SH
    big[d < 0.5] = OUT
    big[(d < 0.6) & (noise > 0.62)] = LAMP_R
    return [big[y * T:(y + 1) * T, x * T:(x + 1) * T] for y in range(size) for x in range(size)]


def hull():
    """The tiles and the 8 x 8 map, sockets drawn in; then crater tiles."""
    img = station_image()
    for name, (c, r, w, h) in PARTS.items():
        y0, x0 = r * T, c * T
        img[y0:y0 + h * T, x0:x0 + w * T] = _socket(w, img[y0:y0 + h * T, x0:x0 + w * T])
    tiles, ids, seen = [np.zeros((T, T), dtype=np.uint8)], {}, {bytes(T * T): 0}
    m = np.zeros((H, W), dtype=np.uint8)
    for r in range(H):
        for c in range(W):
            t = img[r * T:(r + 1) * T, c * T:(c + 1) * T].copy()
            b = t.tobytes()
            if b not in seen:
                seen[b] = len(tiles)
                tiles.append(t)
            m[r, c] = seen[b]
    ids["crater_big0"] = len(tiles)
    tiles += _crater(2, 1)
    ids["crater"] = len(tiles)
    tiles += _crater(1, 2)
    return tiles, ids, m


def core():
    """32x32 hub core: a magenta reactor eye in an armoured collar. Frames: pulse."""
    out = []
    for hot in (False, True):
        s = Sprite(32, 32)
        s.ellipse((1, 1, 31, 31), "armor", amp=2.4)
        for a in range(0, 360, 90):
            x, y = 16 + math.cos(math.radians(a + 45)) * 11, 16 + math.sin(math.radians(a + 45)) * 11
            s.ellipse((x - 3, y - 3, x + 3, y + 3), "vent", z=2.5, amp=0.4, profile="flat")
        s.ellipse((7, 7, 25, 25), "gold", z=2.5, amp=1.2)
        s.ellipse((9.5, 9.5, 22.5, 22.5), "corehot" if hot else "core", z=4)
        s.ellipse((13, 13, 19, 19), "lightb" if hot else "corehot", z=5)
        out.append(s.render(MATS, gloss=0.4))
    return out


def satellite():
    """32x32 orbiting gun satellite: a pod with two small wings. Frames: two
    turns of its spin."""
    out = []
    for k in range(2):
        s = Sprite(32, 32)
        a = k * math.pi / 4
        for side in (-1, 1):
            cx, cy = 16 + side * math.cos(a) * 11, 16 + side * math.sin(a) * 11
            dx, dy = math.cos(a) * 5, math.sin(a) * 5
            px, py = -math.sin(a) * 4, math.cos(a) * 4
            s.poly([(cx - dx - px, cy - dy - py), (cx + dx - px, cy + dy - py), (cx + dx + px, cy + dy + py),
                    (cx - dx + px, cy - dy + py)], "vent", z=1, amp=0.6, bevel=1)
        s.ellipse((8, 8, 24, 24), "armor", z=2, amp=2.2)
        s.ellipse((11, 11, 21, 21), "gold", z=3, amp=1)
        s.ellipse((13.5, 13.5, 18.5, 18.5), "light", z=4)
        out.append(s.render(MATS, gloss=0.35))
    return out


def portrait_art():
    """The station small, in the BOSS6 palette, for the HUD portrait."""
    s = Sprite(32, 32)
    yy, xx = np.mgrid[0:32, 0:32]
    d = np.sqrt((xx - 15.5) ** 2 + (yy - 15.5) ** 2)
    s.mask((d < 15.5) & (d > 10.5), "armor", amp=1.5, bevel=2)
    s.ellipse((10, 10, 22, 22), "gold", z=1, amp=1)
    s.ellipse((13, 13, 19, 19), "core", z=2)
    return s.render(MATS, gloss=0.3)
