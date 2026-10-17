"""Player ship and its effects."""

import numpy as np

from kit import Material, Sprite, value_noise

PLAYER_MATS = {
    "hull": Material([2, 3, 4, 5]),
    "dark": Material([1, 2, 3]),
    "blue": Material([6, 7, 8]),
    "canopy": Material([9, 10, 5], dither=0.2),
    "trim": Material([15], shaded=False),
    "glow": Material([12, 13], shaded=False),
}


def player_sprite() -> Sprite:
    s = Sprite(24, 24)
    # Main wings, swept forward-delta, with drooping tips.
    s.poly([(10, 9), (3, 16), (1, 20), (2, 21), (10, 18)], "hull", z=0, amp=1.2, bevel=2, sym=True)
    s.poly([(9, 12), (4, 16.5), (4, 18), (9, 16.5)], "blue", z=0.6, amp=0.6, bevel=1, sym=True)
    # Wingtip vulcan pods.
    s.rect((1, 12, 3, 20), "dark", z=0.5, amp=0.8, bevel=1, sym=True)
    s.dot(1, 11, "glow", z=1, sym=True)
    # Engine nacelles.
    s.rect((7, 14, 10, 23), "hull", z=1.0, amp=1.5, bevel=1.5, sym=True)
    s.rect((7.5, 21, 9.5, 23), "dark", z=1.0, amp=0.5, bevel=1, sym=True)
    # Fuselage spine.
    s.poly([(12, 0), (14, 5), (15, 13), (14.5, 21), (9.5, 21), (9, 13), (10, 5)], "hull", z=1.5, amp=2.6, bevel=3)
    # Canards.
    s.poly([(10, 6), (6.5, 9.5), (6.5, 10.5), (10, 9.5)], "blue", z=1.2, amp=0.8, bevel=1, sym=True)
    # Cockpit.
    s.ellipse((10, 4, 14, 11), "canopy", z=3.0, amp=1.2)
    # Spine stripe and trim.
    s.line((12, 12), (12, 19), "blue", width=1, z=3)
    s.dot(9, 19, "trim", z=2, sym=True)
    return s


def player_frames():
    """Center, bank 1, bank 2 (rolling right). Left bank is the same frames flipped."""
    s = player_sprite()
    return [s.render(PLAYER_MATS, roll=r, gloss=0.4) for r in (0.0, 0.38, 0.72)]


def flame_frames():
    """8x8 engine flame below each nacelle pair, 4 frames of flicker.

    Uses the player palette's engine entries 11-14 (11 red outer, 14 white core).
    """
    frames = []
    lengths = [6, 8, 5, 7]
    for i, ln in enumerate(lengths):
        f = np.zeros((8, 8), dtype=np.uint8)
        for y in range(ln):
            t = y / max(1, ln - 1)
            half = 1.6 * (1 - t) + 0.4
            for x in range(8):
                dx = abs(x - 3.5)
                if dx <= half + 0.5:
                    core = dx / (half + 0.5)
                    v = (1 - t) * 0.7 + (1 - core) * 0.6
                    if v > 0.95:
                        f[y, x] = 14
                    elif v > 0.7:
                        f[y, x] = 13
                    elif v > 0.45:
                        f[y, x] = 12
                    else:
                        f[y, x] = 11
        frames.append(f)
    return frames
