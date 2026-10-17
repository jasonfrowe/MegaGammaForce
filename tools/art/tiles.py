"""Stage 1 background: 16x16 tiles (sea, fortress deck, transitions) and the stage map.

The deck/sea boundary uses marching squares over a grid of corner values,
so any arrangement of deck corners has a matching tile. Inside a tile the
edge follows the bilinear blend of its four corners, which is continuous
across tile borders. Fully-deck tiles get detail variants (plates, vents,
lamps, hazard stripes, pipes, big hatches).

The map is written top row first, the way Mode 2 reads it. Its top
LOOP_ROWS*2 rows are two identical copies of a sea segment, so the game
can wrap the scroll there forever during the boss fight.
"""

import math

import numpy as np

from kit import value_noise

T = 16
COLS = 12
LOOP_ROWS = 16

# Palette indices (palettes.TILES).
SEA = [1, 2, 3, 4]
DECK = [5, 6, 7, 8, 9]
SHADOW, HAZ0, HAZ1, LAMP_R, LAMP_G, FOAM = 10, 11, 12, 13, 14, 15


def _torus_noise(size, cell, seed):
    """Value noise that tiles seamlessly on a size x size torus."""
    rng = np.random.default_rng(seed)
    n = size // cell
    g = rng.random((n, n))
    yy, xx = np.mgrid[0:size, 0:size] / cell
    x0, y0 = xx.astype(int), yy.astype(int)
    fx, fy = xx - x0, yy - y0
    fx, fy = fx * fx * (3 - 2 * fx), fy * fy * (3 - 2 * fy)
    x1, y1 = (x0 + 1) % n, (y0 + 1) % n
    a = g[y0, x0] * (1 - fx) + g[y0, x1] * fx
    b = g[y1, x0] * (1 - fx) + g[y1, x1] * fx
    return a * (1 - fy) + b * fy


def sea_tile(variant):
    n = _torus_noise(T, 4, 10 + variant) * 0.6 + _torus_noise(T, 8, 20 + variant) * 0.4
    yy, xx = np.mgrid[0:T, 0:T]
    # Long diagonal swell bands.
    band = (np.sin((xx * 0.4 + yy * 0.9 + variant * 3) * 0.8) + 1) / 2
    v = n * 0.7 + band * 0.3
    t = np.zeros((T, T), dtype=np.uint8)
    t[:] = SEA[0]
    t[v > 0.38] = SEA[1]
    t[v > 0.62] = SEA[2]
    t[v > 0.78] = SEA[3]
    return t


def deck_plate(seed=0):
    yy, xx = np.mgrid[0:T, 0:T]
    n = value_noise(T, T, 4, 50 + seed)
    t = np.full((T, T), DECK[2], dtype=np.uint8)
    t[n > 0.62] = DECK[3]
    t[n < 0.30] = DECK[1]
    # Plate seams on the tile border: dark at the bottom/right, light at the top/left.
    t[0, :] = DECK[3]
    t[:, 0] = DECK[3]
    t[T - 1, :] = DECK[0]
    t[:, T - 1] = DECK[0]
    return t


def deck_variant(kind):
    t = deck_plate(kind)
    if kind == 1:  # rivets
        for (x, y) in ((3, 3), (12, 3), (3, 12), (12, 12)):
            t[y, x] = DECK[4]
            t[y + 1, x + 1] = DECK[0]
    elif kind == 2:  # vent grille
        t[3:13, 3:13] = SHADOW
        for y in range(4, 13, 2):
            t[y, 4:12] = DECK[1]
        t[3, 3:13] = DECK[0]
        t[13, 3:14] = DECK[4]
        t[3:14, 13] = DECK[4]
    elif kind == 3:  # red lamp
        t[5:11, 5:11] = DECK[0]
        t[6:10, 6:10] = LAMP_R
        t[6, 6] = t[6, 9] = t[9, 6] = t[9, 9] = DECK[0]
        t[7, 7] = FOAM
    elif kind == 4:  # hazard stripes
        yy, xx = np.mgrid[0:T, 0:T]
        stripe = ((xx + yy) // 4) % 2 == 0
        t[2:14, :] = np.where(stripe[2:14, :], HAZ0, SHADOW)
        t[2, :] = DECK[4]
        t[13, :] = DECK[0]
    elif kind == 5:  # horizontal pipe
        t[5:11, :] = DECK[1]
        t[6, :] = DECK[3]
        t[7, :] = DECK[4]
        t[8, :] = DECK[2]
        t[10, :] = SHADOW
        t[5:11, 7:9] = HAZ1
    elif kind == 6:  # vertical pipe
        t[:, 5:11] = DECK[1]
        t[:, 6] = DECK[3]
        t[:, 7] = DECK[4]
        t[:, 8] = DECK[2]
        t[:, 10] = SHADOW
    elif kind == 7:  # green status lamp strip
        t[6:10, 2:14] = DECK[0]
        for x in range(3, 13, 3):
            t[7:9, x:x + 2] = LAMP_G
    return t


def hatch_tiles():
    """A 32x32 circular hatch, as 4 tiles: TL, TR, BL, BR."""
    big = np.zeros((32, 32), dtype=np.uint8)
    for ty in range(2):
        for tx in range(2):
            big[ty * T:(ty + 1) * T, tx * T:(tx + 1) * T] = deck_plate(9)
    yy, xx = np.mgrid[0:32, 0:32]
    d = np.sqrt((xx - 15.5) ** 2 + (yy - 15.5) ** 2)
    ang = np.arctan2(yy - 15.5, xx - 15.5)
    big[d < 14] = DECK[1]
    big[(d < 14) & (d > 12.5) & (np.cos(ang + 0.8) > 0)] = DECK[4]
    big[(d < 14) & (d > 12.5) & (np.cos(ang + 0.8) <= 0)] = DECK[0]
    big[d < 11] = DECK[2]
    seg = ((ang + math.pi) / (2 * math.pi) * 8).astype(int) % 2 == 0
    big[(d < 10.5) & (d > 3) & seg] = DECK[3]
    big[(d < 10.5) & (np.abs(np.sin(ang * 4)) < 0.08)] = SHADOW
    big[d < 3.5] = HAZ0
    big[d < 2] = LAMP_R
    return [big[0:16, 0:16], big[0:16, 16:32], big[16:32, 0:16], big[16:32, 16:32]]


def edge_tile(c):
    """Marching squares tile for corners (tl, tr, br, bl) of 0 = sea, 1 = deck."""
    tl, tr, br, bl = c
    yy, xx = np.mgrid[0:T, 0:T]
    u = (xx + 0.5) / T
    v = (yy + 0.5) / T
    f = tl * (1 - u) * (1 - v) + tr * u * (1 - v) + br * u * v + bl * (1 - u) * v
    t = sea_tile(0)
    plate = deck_plate(0)
    deck = f >= 0.5
    t[deck] = plate[deck]
    t[(f >= 0.5) & (f < 0.56)] = DECK[4]   # lit rim
    t[(f >= 0.56) & (f < 0.63)] = DECK[0]  # bevel shadow
    foam = (f < 0.5) & (f >= 0.43)
    t[foam] = FOAM
    t[(f < 0.43) & (f >= 0.38)] = SHADOW
    return t


def build_tileset():
    """Returns (tiles list, ids dict)."""
    tiles = []
    ids = {}

    def add(name, tile):
        ids[name] = len(tiles)
        tiles.append(tile)

    for k in range(4):
        add(f"sea{k}", sea_tile(k))
    for cfg in range(1, 15):
        c = ((cfg >> 3) & 1, (cfg >> 2) & 1, (cfg >> 1) & 1, cfg & 1)
        add(f"edge{cfg}", edge_tile(c))
    for k in range(8):
        add(f"deck{k}", deck_variant(k))
    for k, h in enumerate(hatch_tiles()):
        add(f"hatch{k}", h)
    return tiles, ids


def stage_corners(rows, seed=1):
    """Deck coverage per corner, stage order (row 0 = first seen, at the bottom)."""
    rng = np.random.default_rng(seed)
    n = value_noise(COLS + 1, rows + 1, 3, seed) * 0.6 + value_noise(COLS + 1, rows + 1, 6, seed + 1) * 0.4
    corners = np.zeros((rows + 1, COLS + 1), dtype=np.uint8)
    for r in range(rows + 1):
        t = r / rows
        # Coverage target ramps up into the fortress, then falls away before the boss.
        if t < 0.08:
            cov = 0.0
        elif t < 0.30:
            cov = (t - 0.08) / 0.22 * 0.45
        elif t < 0.45:
            cov = 0.45 + (t - 0.30) / 0.15 * 0.35
        elif t < 0.80:
            cov = 0.80 + 0.12 * math.sin(t * 40)
        elif t < 0.88:
            cov = 0.80 - (t - 0.80) / 0.08 * 0.80
        else:
            cov = 0.0
        for c in range(COLS + 1):
            # A central runway keeps the fortress readable.
            centre = 1.0 - abs(c - COLS / 2) / (COLS / 2)
            v = n[r, c] * 0.75 + centre * 0.25
            corners[r, c] = 1 if v > 1.0 - cov else 0
    return corners


def build_map(rows, ids, seed=1):
    """Tile ids, stage order. Rows past the deck section are sea."""
    corners = stage_corners(rows, seed)
    rng = np.random.default_rng(seed + 7)
    m = np.zeros((rows, COLS), dtype=np.uint8)
    hatch_used = np.zeros((rows, COLS), dtype=bool)
    hatches = []
    for r in range(rows):
        for c in range(COLS):
            # Stage order: row r's "top" corners are row r+1 (it scrolls in from above).
            tl, tr = corners[r + 1, c], corners[r + 1, c + 1]
            bl, br = corners[r, c], corners[r, c + 1]
            cfg = (tl << 3) | (tr << 2) | (br << 1) | bl
            if cfg == 0:
                m[r, c] = ids[f"sea{int(rng.integers(0, 4))}"]
            elif cfg == 15:
                roll = rng.random()
                if roll < 0.70:
                    k = 0
                elif roll < 0.86:
                    k = 1
                elif roll < 0.91:
                    k = 2
                elif roll < 0.95:
                    k = 3
                elif roll < 0.98:
                    k = 7
                else:
                    k = 6
                m[r, c] = ids[f"deck{k}"]
            else:
                m[r, c] = ids[f"edge{cfg}"]
    # Pipes along long runs, hazard stripes across the runway, hatches on 2x2 deck.
    for r in range(1, rows - 1):
        for c in range(COLS - 1):
            full = all(m[r + dy, c + dx] in (ids["deck0"], ids["deck1"]) for dy in (0, 1) for dx in (0, 1))
            if full and not hatch_used[r:r + 2, c:c + 2].any() and rng.random() < 0.06:
                # Stage order is bottom-up, so the hatch's top row is r+1.
                m[r + 1, c], m[r + 1, c + 1] = ids["hatch0"], ids["hatch1"]
                m[r, c], m[r, c + 1] = ids["hatch2"], ids["hatch3"]
                hatch_used[r:r + 2, c:c + 2] = True
                hatches.append((r, c))
    for r in range(rows):
        if r % 23 == 11:
            run = [c for c in range(COLS) if m[r, c] in (ids["deck0"], ids["deck1"])]
            if len(run) >= 6:
                for c in run:
                    m[r, c] = ids["deck4"]
        if r % 17 == 5:
            for c in range(COLS):
                if m[r, c] in (ids["deck0"], ids["deck1"]):
                    m[r, c] = ids["deck5"]
    return m, hatches


def stage1(total_rows=208):
    tiles, ids = build_tileset()
    body_rows = total_rows - 2 * LOOP_ROWS
    body, hatches = build_map(body_rows, ids)
    rng = np.random.default_rng(99)
    loop = np.array([[ids[f"sea{int(rng.integers(0, 4))}"] for _ in range(COLS)] for _ in range(LOOP_ROWS)],
                    dtype=np.uint8)
    stage = np.concatenate([body, loop, loop], axis=0)  # stage order
    top_first = stage[::-1].copy()
    return tiles, ids, top_first, hatches
