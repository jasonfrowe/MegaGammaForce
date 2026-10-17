#!/usr/bin/env python3
"""Generate MegaGammaForce's OPL2 sound effects.

Adapted from RPDemo's tools/generate_sfx.py. Each effect is a short stream
of OPL2 register writes and waits (0x5A reg val, 0x61 wait16, 0x66 end),
on channel 7 (player) or 8 (enemy); music never uses those two channels.
All the streams are packed into one blob that the ROM loads into XRAM.

Writes:
  music/sfx.bin          the blob, loaded at XRAM_SFX_DATA
  music/sfx/<name>.vgm   each effect alone, for auditioning
  src/gen/sfx_ids.h      an id per effect and its offset in the blob

Usage: python3 tools/gen_sfx.py
"""

import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
VGM_RATE = 44100
OPL_RATE = 49716.0
MOD = {7: 0x11, 8: 0x12}
CAR = {7: 0x14, 8: 0x15}


def fnum_block(hz):
    for block in range(8):
        f = round(hz * (2 ** (20 - block)) / OPL_RATE)
        if 0 < f <= 1023:
            return f, block
    return 1023, 7


class Sfx:
    def __init__(self, ch):
        self.ch = ch
        self.ev = []  # (reg, val, wait)

    def w(self, reg, val):
        self.ev.append((reg, val & 0xFF, 0))

    def patch(self, mod_mult=1, mod_tl=0, mod_ar=15, mod_dr=8, mod_sl=0, mod_rr=8, mod_wave=0,
              car_mult=1, car_tl=0, car_ar=15, car_dr=8, car_sl=0, car_rr=8, car_wave=0,
              feedback=0, connection=0, sustain=1):
        m, c = MOD[self.ch], CAR[self.ch]
        self.w(0x20 + m, (sustain << 5) | mod_mult)
        self.w(0x20 + c, (sustain << 5) | car_mult)
        self.w(0x40 + m, mod_tl)
        self.w(0x40 + c, car_tl)
        self.w(0x60 + m, (mod_ar << 4) | mod_dr)
        self.w(0x60 + c, (car_ar << 4) | car_dr)
        self.w(0x80 + m, (mod_sl << 4) | mod_rr)
        self.w(0x80 + c, (car_sl << 4) | car_rr)
        self.w(0xE0 + m, mod_wave)
        self.w(0xE0 + c, car_wave)
        self.w(0xC0 + self.ch, (feedback << 1) | connection)
        return self

    def on(self, hz):
        f, b = fnum_block(hz)
        self.w(0xA0 + self.ch, f & 0xFF)
        self.w(0xB0 + self.ch, 0x20 | (b << 2) | (f >> 8))
        return self

    def off(self):
        self.w(0xB0 + self.ch, 0)
        return self

    def wait(self, ms):
        n = round(ms * VGM_RATE / 1000)
        if n > 0:
            r, v, w = self.ev[-1]
            self.ev[-1] = (r, v, w + n)
        return self

    def sweep(self, a, b, ms, steps=10):
        for i in range(steps + 1):
            self.on(a + (b - a) * i / steps)
            if i < steps:
                self.wait(ms / steps)
        return self

    def tl(self, level):
        self.w(0x40 + CAR[self.ch], level)
        return self

    def stream(self):
        out = bytearray()
        for r, v, w in self.ev:
            out += bytes([0x5A, r, v])
            while w > 0:
                n = min(w, 0xFFFF)
                out += bytes([0x61]) + struct.pack("<H", n)
                w -= n
        out.append(0x66)
        return bytes(out)


# Laser-ish chirps keep feedback low so they sit cleanly over the music.
def vulcan():
    return Sfx(7).patch(mod_tl=30, mod_dr=11, car_dr=10, car_sl=3, car_rr=11, feedback=1).sweep(1400, 700, 40, 5).off()


def homing():
    return Sfx(7).patch(mod_mult=2, mod_tl=24, car_dr=8, car_sl=3, feedback=3, mod_wave=1).sweep(500, 1200, 80, 8).off()


def spread():
    return Sfx(7).patch(mod_mult=3, mod_tl=22, mod_wave=2, car_dr=10, car_sl=4, car_rr=10, feedback=4).sweep(900, 400, 60, 6).off()


def plasma():
    s = Sfx(7).patch(mod_mult=7, mod_tl=12, mod_wave=3, car_mult=2, car_tl=6, car_dr=12, car_sl=6, feedback=7)
    for f in (1800, 1300, 2100, 1500):
        s.on(f).wait(12)
    return s.off()


def sword():
    return Sfx(7).patch(mod_mult=1, mod_tl=18, mod_wave=1, car_mult=2, car_dr=6, car_sl=2, car_rr=6, feedback=5).sweep(300, 900, 120, 10).off()


def hyper():
    s = Sfx(7).patch(mod_mult=1, mod_tl=8, mod_wave=1, car_mult=1, car_dr=3, car_sl=1, car_rr=4, feedback=6)
    s.sweep(150, 1600, 450, 24)
    s.sweep(1600, 1200, 200, 8)
    return s.off()


def player_die():
    return Sfx(7).patch(mod_mult=5, mod_tl=8, mod_dr=8, mod_sl=6, mod_rr=6, mod_wave=3, car_mult=2, car_dr=7, car_sl=5,
                        car_rr=5, car_wave=3, feedback=7).sweep(700, 50, 700, 22).off()


def chain():
    """Chain set complete: quick rising arpeggio, additive chime."""
    s = Sfx(7).patch(mod_tl=6, mod_dr=6, mod_sl=4, mod_rr=6, car_mult=2, car_dr=5, car_sl=5, car_rr=5, feedback=3,
                     connection=1)
    for f in (1046.5, 1318.5, 1568.0, 2093.0):
        s.on(f).wait(45).off().wait(8)
    return s


def absorb():
    return Sfx(7).patch(mod_mult=4, mod_tl=26, car_mult=4, car_tl=6, car_dr=12, car_sl=6, feedback=2).sweep(2400, 3200, 30, 4).off()


def clear():
    s = Sfx(7).patch(mod_tl=4, mod_dr=6, mod_sl=4, mod_rr=6, car_mult=2, car_dr=5, car_sl=5, car_rr=5, feedback=3,
                     connection=1)
    for f in (523.25, 659.25, 783.99, 1046.5, 783.99, 1046.5):
        s.on(f).wait(130).off().wait(20)
    s.on(1318.5)
    for _ in range(8):
        for lv in (0, 2, 5, 9, 14, 9, 5, 2):
            s.tl(lv).wait(10)
    return s.off()


def enemy_fire():
    return Sfx(8).patch(mod_tl=26, mod_dr=10, mod_rr=10, car_dr=9, car_sl=2, car_rr=10, feedback=1).sweep(450, 200, 90, 8).off()


def explode():
    return Sfx(8).patch(mod_mult=4, mod_tl=10, mod_dr=12, mod_sl=4, mod_rr=12, mod_wave=3, car_mult=3, car_dr=11,
                        car_sl=3, car_rr=11, car_wave=3, feedback=7).sweep(500, 120, 160, 10).off()


def big_explode():
    return Sfx(8).patch(mod_mult=6, mod_tl=6, mod_dr=7, mod_sl=5, mod_rr=5, mod_wave=3, car_mult=1, car_dr=6, car_sl=4,
                        car_rr=4, car_wave=3, feedback=7).sweep(380, 40, 600, 20).off()


def warning():
    s = Sfx(8).patch(mod_mult=2, mod_tl=14, mod_wave=1, car_mult=1, car_dr=2, car_sl=1, car_rr=8, car_wave=1, feedback=4)
    for _ in range(2):
        s.sweep(440, 880, 250, 10).sweep(880, 440, 250, 10)
    return s.off()


def boss_hit():
    return Sfx(8).patch(mod_mult=1, mod_tl=20, mod_wave=2, car_dr=14, car_sl=8, car_rr=14, feedback=6).sweep(260, 180, 30, 3).off()


SFX = [
    ("VULCAN", vulcan),
    ("HOMING", homing),
    ("SPREAD", spread),
    ("PLASMA", plasma),
    ("SWORD", sword),
    ("HYPER", hyper),
    ("PLAYER_DIE", player_die),
    ("CHAIN", chain),
    ("ABSORB", absorb),
    ("CLEAR", clear),
    ("ENEMY_FIRE", enemy_fire),
    ("EXPLODE", explode),
    ("BIG_EXPLODE", big_explode),
    ("WARNING", warning),
    ("BOSS_HIT", boss_hit),
]


def main():
    blob = bytearray()
    offsets = []
    (ROOT / "music" / "sfx").mkdir(parents=True, exist_ok=True)
    for name, fn in SFX:
        data = fn().stream()
        offsets.append(len(blob))
        blob += data
        hdr = bytearray(0x40)
        hdr[0:4] = b"Vgm "
        (ROOT / "music" / "sfx" / f"{name.lower()}.vgm").write_bytes(bytes(hdr) + data)
    if len(blob) & 1:
        blob.append(0)
    (ROOT / "music" / "sfx.bin").write_bytes(bytes(blob))
    h = ["/* Generated by tools/gen_sfx.py. Do not edit; regenerate instead. */",
         "#ifndef GEN_SFX_IDS_H", "#define GEN_SFX_IDS_H", "", "enum", "{"]
    h += [f"    SFX_{name}," for name, _ in SFX]
    h += ["    SFX_COUNT,", "};", "", f"#define SFX_BLOB_SIZE {len(blob)}", "",
          "#ifdef SFX_OFFSETS", "static const uint16_t sfx_offsets[SFX_COUNT] = {"]
    h += [f"    {o}, /* {name} */" for o, (name, _) in zip(offsets, SFX)]
    h += ["};", "#endif", "", "#endif", ""]
    (ROOT / "src" / "gen" / "sfx_ids.h").write_text("\n".join(h))
    print(f"{len(SFX)} sfx, {len(blob)} bytes")


if __name__ == "__main__":
    main()
