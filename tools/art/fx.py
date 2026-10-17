"""Projectiles, the radiant sword and explosions: procedural, radial art."""

import math

import numpy as np

from kit import value_noise


def _radial(size, cx, cy, r, ramp, sx=1.0, sy=1.0):
    """Glowing blob: ramp[0] at the rim to ramp[-1] at the center."""
    f = np.zeros((size[1], size[0]), dtype=np.uint8)
    yy, xx = np.mgrid[0:size[1], 0:size[0]]
    d = np.sqrt(((xx + 0.5 - cx) / sx) ** 2 + ((yy + 0.5 - cy) / sy) ** 2) / r
    for i, idx in enumerate(ramp):
        t = 1.0 - i / len(ramp)
        f[d <= t] = idx
    return f


def vulcan_frames():
    """8x16: twin bolts. Shots palette 2-5."""
    out = []
    for k in range(2):
        f = np.zeros((16, 8), dtype=np.uint8)
        for x0 in (1, 5):
            f[1:15, x0:x0 + 2] = 3
            f[0 + k:14, x0:x0 + 2] = 4
            f[2 + k:11, x0] = 5
            f[14:16, x0:x0 + 2] = 2
        out.append(f)
    return out


def shot8_frames():
    """8x8 player shots. Order: spread x2, homing x2, spark x2, back x2."""
    out = []
    # Spread: orange orb, pulsing.
    out.append(_radial((8, 8), 4, 4, 3.6, [6, 7, 8, 9]))
    out.append(_radial((8, 8), 4, 4, 3.2, [7, 8, 9, 9]))
    # Homing: green diamond with a bright core.
    for k in range(2):
        f = np.zeros((8, 8), dtype=np.uint8)
        yy, xx = np.mgrid[0:8, 0:8]
        d = np.abs(xx - 3.5) + np.abs(yy - 3.5)
        f[d <= 4.0 - k * 0.5] = 10
        f[d <= 3.0 - k * 0.5] = 11
        f[d <= 2.0] = 12
        f[d <= 1.0] = 13
        out.append(f)
    # Spark (plasma link): violet crackle.
    rng = np.random.default_rng(7)
    for k in range(2):
        f = np.zeros((8, 8), dtype=np.uint8)
        x, y = 3, 0
        for step in range(8):
            f[y, max(0, min(7, x))] = 15
            if 0 < x < 7:
                f[y, x - 1] = max(f[y, x - 1], 14)
                f[y, x + 1] = max(f[y, x + 1], 14)
            x += int(rng.integers(-1, 2))
            y += 1
            if y > 7:
                break
        out.append(f)
    # Back shot: cyan lozenge.
    out.append(_radial((8, 8), 4, 4, 4.0, [2, 3, 4, 5], sx=0.6))
    out.append(_radial((8, 8), 4, 4, 3.5, [3, 4, 5], sx=0.6))
    return out


def sword_frames():
    """48x32 radiant sword sweep above the ship (ship at bottom center).

    A bright arc of blade energy, rotating through 4 frames.
    """
    w, h = 48, 32
    out = []
    yy, xx = np.mgrid[0:h, 0:w]
    cx, cy = 24, 34
    ang = np.arctan2(-(yy + 0.5 - cy), xx + 0.5 - cx)  # 0 right, pi/2 up
    rad = np.sqrt((xx + 0.5 - cx) ** 2 + (yy + 0.5 - cy) ** 2)
    for k in range(4):
        f = np.zeros((h, w), dtype=np.uint8)
        lead = math.pi * (0.15 + 0.7 * ((k + 1) / 4.0))  # sweep left to right? rotate leading edge
        # Blade arc spans 150 degrees with a trailing fade.
        span = math.radians(150)
        a_rel = (ang - (math.pi / 2 - span / 2)) / span  # 0..1 across the arc
        inside = (a_rel >= 0) & (a_rel <= 1)
        phase = (a_rel * 4 - k) % 4 / 4.0  # moving streaks
        ring = np.abs(rad - 24) / 7.5
        core = inside & (ring < 1.0)
        f[core & (ring < 1.0)] = 2
        f[core & (ring < 0.75)] = 3
        f[core & (ring < 0.45)] = 4
        f[core & (ring < 0.2)] = 5
        streak = core & (phase < 0.06) & (ring < 0.55)
        f[streak] = 5
        # Inner shimmer sparks.
        rng = np.random.default_rng(100 + k)
        for _ in range(10):
            a = math.pi / 2 + (rng.random() - 0.5) * span
            r = 12 + rng.random() * 8
            x = int(cx + math.cos(a) * r)
            y = int(cy - math.sin(a) * r)
            if 0 <= x < w and 0 <= y < h:
                f[y, x] = 15 if rng.random() < 0.5 else 4
        out.append(f)
    return out


def bullet8_frames():
    """8x8 enemy bullets, palette 1-5. Order: small orb x2, needle x2."""
    out = [
        _radial((8, 8), 4, 4, 3.2, [1, 2, 3, 4, 5]),
        _radial((8, 8), 4, 4, 3.6, [1, 2, 3, 4, 5, 5]),
    ]
    for k in range(2):
        f = np.zeros((8, 8), dtype=np.uint8)
        f[0:8, 3:5] = 2
        f[1:7, 3:5] = 3
        f[2 + k:6, 3:5] = 4
        f[3:5, 3:5] = 5
        f[1:7, 2] = 1
        f[1:7, 5] = 1
        out.append(f)
    return out


def bullet16_frames():
    """16x16 big orbs, 2 pulse frames."""
    return [
        _radial((16, 16), 8, 8, 7.0, [1, 2, 3, 3, 4, 5]),
        _radial((16, 16), 8, 8, 7.6, [1, 2, 2, 3, 4, 4, 5]),
    ]


def explosion_frames(size, seed, count=6):
    """Fireball that blooms then breaks into smoke. Explosion palette."""
    out = []
    n1 = value_noise(size, size, max(2, size // 4), seed)
    n2 = value_noise(size, size, max(2, size // 8), seed + 1)
    noise = n1 * 0.65 + n2 * 0.35
    yy, xx = np.mgrid[0:size, 0:size]
    c = size / 2.0
    d = np.sqrt((xx + 0.5 - c) ** 2 + (yy + 0.5 - c) ** 2) / c
    for k in range(count):
        t = k / (count - 1)
        radius = 0.35 + 0.65 * math.sqrt(t)
        heat = 1.0 - t
        f = np.zeros((size, size), dtype=np.uint8)
        v = (1 - d / radius) + (noise - 0.5) * 0.9
        body = v > 0.05
        hole = (t > 0.45) & ((1 - d / radius) + (noise - 0.5) * 1.2 > 0.55 + (1 - t))
        temp = v * heat * 2.2
        ramp = [1, 2, 3, 4, 5, 6, 7, 8]
        lvl = np.clip((temp * (len(ramp) - 1)).astype(int), 0, len(ramp) - 1)
        f[body] = np.array(ramp)[lvl[body]]
        # Smoke takes over as it cools.
        smoke = body & (temp < 0.25) & (t > 0.3)
        f[smoke] = np.where(noise[smoke] > 0.5, 10, 9)
        f[hole & (t > 0.6)] = 0
        if k == 0:
            f[d < 0.35] = 8
            f[(d < 0.6) & (f == 0) & (noise > 0.4)] = 7
        out.append(f)
    return out


def beam_frames():
    """32x64 hyper sword beam segment, tiles vertically. Shots palette."""
    out = []
    yy, xx = np.mgrid[0:64, 0:32]
    for k in range(2):
        f = np.zeros((64, 32), dtype=np.uint8)
        wob = np.sin((yy + k * 8) * 2 * math.pi / 32) * 1.5 + np.sin((yy * 3 + k * 20) * 2 * math.pi / 64) * 1.0
        d = np.abs(xx + 0.5 - 16 - wob * 0.6)
        f[d < 13] = 1
        f[d < 11] = 2
        f[d < 8] = 3
        f[d < 5] = 4
        f[d < 2.5] = 5
        rng = np.random.default_rng(300 + k)
        for _ in range(26):
            x = int(rng.integers(1, 31))
            y = int(rng.integers(0, 64))
            if f[y, x] < 3:
                f[y, x] = 15 if rng.random() < 0.5 else 14
        out.append(f)
    return out
