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

    def modtl(self, level):
        """Modulator level: lower is brighter, higher is darker and purer."""
        self.w(0x40 + MOD[self.ch], level)
        return self

    def boom(self, a, b, ms, steps, tl0, tl1):
        """Pitch drop that also darkens as it falls: the body of an explosion."""
        for i in range(steps + 1):
            t = i / steps
            self.modtl(round(tl0 + (tl1 - tl0) * t))
            self.on(a + (b - a) * t)
            if i < steps:
                self.wait(ms / steps)
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


# Everything here is tonal, for a full sound under the orchestral music:
# sine carriers, modulator multiples of 1-3 (saw- and square-like
# harmonics), feedback no higher than 5. RPDemo's noise recipe (pulse-sine
# waveforms with feedback 7) comes out as thin scratching on this mix.
# Explosions are low booms whose brightness fades as the pitch falls.

def vulcan():
    return Sfx(7).patch(mod_tl=18, mod_dr=6, car_dr=6, car_sl=4, car_rr=8, feedback=4).sweep(880, 330, 60, 6).off()


def homing():
    return (Sfx(7).patch(mod_mult=2, mod_tl=14, mod_dr=5, car_dr=5, car_sl=3, car_rr=7, feedback=2, connection=1)
            .sweep(330, 990, 80, 8).off())


def spread():
    return Sfx(7).patch(mod_tl=14, mod_dr=5, car_dr=5, car_sl=4, car_rr=7, feedback=5).sweep(520, 180, 90, 8).off()


def plasma():
    s = Sfx(7).patch(mod_mult=3, mod_tl=20, car_dr=4, car_sl=2, car_rr=6, feedback=4)
    for f in (220, 233, 220, 247, 220):
        s.on(f).wait(14)
    return s.off()


def sword():
    s = Sfx(7).patch(mod_tl=22, car_ar=12, car_dr=3, car_sl=2, car_rr=6, feedback=3)
    return s.sweep(180, 520, 90, 8).sweep(520, 260, 80, 6).off()


def hyper():
    s = Sfx(7).patch(mod_tl=10, car_dr=2, car_sl=1, car_rr=3, feedback=5)
    s.sweep(110, 880, 500, 24)
    for _ in range(6):
        s.on(860).wait(25).on(900).wait(25)
    return s.off()


def player_die():
    s = Sfx(7).patch(mod_tl=8, mod_dr=2, car_dr=2, car_sl=2, car_rr=3, feedback=5)
    s.boom(160, 150, 30, 2, 4, 6)
    return s.boom(300, 30, 900, 30, 8, 40).off()


def chain():
    """Chain set complete: quick rising arpeggio, additive chime."""
    s = Sfx(7).patch(mod_tl=6, mod_dr=6, mod_sl=4, mod_rr=6, car_mult=2, car_dr=5, car_sl=5, car_rr=5, feedback=3,
                     connection=1)
    for f in (1046.5, 1318.5, 1568.0, 2093.0):
        s.on(f).wait(45).off().wait(8)
    return s


def absorb():
    return (Sfx(7).patch(mod_mult=2, mod_tl=24, car_mult=2, car_dr=8, car_sl=6, feedback=2, connection=1)
            .sweep(1200, 1600, 30, 4).off())


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


def lock():
    """Lock-on blip: two quick rising pips."""
    s = Sfx(7).patch(mod_mult=2, mod_tl=20, car_mult=2, car_dr=8, car_sl=5, car_rr=9, feedback=2, connection=1)
    return s.on(1320).wait(25).off().wait(10).on(1760).wait(30).off()


def extend():
    """Extra life: a bright major fanfare, then a shimmering held top note."""
    s = Sfx(7).patch(mod_mult=1, mod_tl=12, mod_dr=4, car_mult=2, car_dr=3, car_sl=3, car_rr=5, feedback=4,
                     connection=0)
    for f, ms in ((783.99, 70), (1046.5, 70), (1318.5, 70), (1568.0, 70), (1318.5, 70), (1568.0, 70)):
        s.on(f).wait(ms).off().wait(10)
    s.on(2093.0)
    for _ in range(10):
        for lv in (0, 3, 6, 3):
            s.tl(lv).wait(15)
    return s.off()


def enemy_fire():
    return Sfx(8).patch(mod_mult=2, mod_tl=22, car_dr=6, car_sl=4, car_rr=8, feedback=2).sweep(600, 260, 70, 7).off()


def explode():
    s = Sfx(8).patch(mod_tl=6, mod_dr=3, car_dr=4, car_sl=3, car_rr=5, feedback=5)
    s.boom(170, 150, 25, 2, 4, 8)
    return s.boom(140, 45, 280, 14, 10, 36).off()


def big_explode():
    s = Sfx(8).patch(mod_tl=4, mod_dr=2, car_dr=2, car_sl=2, car_rr=3, feedback=5)
    s.boom(150, 130, 30, 2, 2, 6)
    s.boom(120, 40, 350, 14, 8, 30)
    s.boom(110, 28, 450, 16, 10, 42)
    return s.off()


def warning():
    s = Sfx(8).patch(mod_tl=16, car_dr=2, car_sl=1, car_rr=8, feedback=4)
    for _ in range(2):
        s.sweep(440, 880, 250, 10).sweep(880, 440, 250, 10)
    return s.off()


def boss_hit():
    return Sfx(8).patch(mod_mult=3, mod_tl=18, car_dr=9, car_sl=6, car_rr=9, feedback=3).sweep(230, 200, 40, 3).off()


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
    ("LOCK", lock),
    ("EXTEND", extend),
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
