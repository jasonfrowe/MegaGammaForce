#!/usr/bin/env python3
"""Run an emulator script against the built ROM, with symbols filled in.

The emulator's --script language addresses memory by number, and the
numbers move every build. Write {name} in a script instead, and this
fills in the address of that symbol from the ELF. {name+1} works too.

Built-in scripts (pass the name instead of a file):
  boss     warp to the end of the stage, fight the boss invulnerable,
           print dropped frames and save screenshots
  soak     play the whole stage holding fire, invulnerable, printing
           dropped frames as it goes

Usage:
  tools/playtest.py boss [--out DIR] [--stage N]
  tools/playtest.py my_script.txt [--out DIR]
  tools/playtest.py boss --phi2 4000     # the 6502 at half speed: does it still keep up?
"""

import argparse
import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
BUILD = ROOT / "build" / "llvm-mos" / "release"
ROM = BUILD / "MegaGammaForce.rp6502"
ELF = BUILD / "MegaGammaForce.elf"
NM = Path.home() / ".rp6502" / "llvm-mos" / "bin" / "llvm-nm"
EMU = ROOT / "tools" / "rp6502-emu"

KEEP_ALIVE = "poke {invuln} 0xC8\npoke {pl_lives} 3\n"

BUILTIN = {
    "boss": (
        "run 60\npoke {first_stage} {stage}\npress z\nrun 2\nrelease z\nrun 100\n"
        # View near the top of the map, distance near the end, no waves left,
        # the sky (if any) at its empty top.
        "poke {view_top} 0x40 0x01\npoke {bg_distance} 0x00 0x0B\npoke {next_wave} 99\npoke {sky_top} 0 0\n"
        "run 200\nshot \"{out}/boss_warning.png\"\nrun 260\n"
        + "".join(
            f"press {k}\n" + (KEEP_ALIVE + "run 100\n") * 4 + f"dump {{lag_frames}} 2\nshot \"{{out}}/boss_{i}.png\"\nrelease {k}\n"
            for i, k in enumerate("zcxnzcbv")
        )
        + (KEEP_ALIVE + "run 100\n") * 8
        + "shot \"{out}/boss_end.png\"\ndump {lag_frames} 2\ndump {gstate} 1\n"
    ),
    "soak": (
        "run 60\npoke {first_stage} {stage}\npress z\nrun 2\nrelease z\nrun 10\n"
        + "".join(
            f"press {k}\n" + (KEEP_ALIVE + "run 100\n") * 8 + f"dump {{lag_frames}} 2\nshot \"{{out}}/soak_{i}.png\"\nrelease {k}\n"
            for i, k in enumerate("zxcvbnzc")
        )
    ),
}


def symbols():
    out = subprocess.run([str(NM), str(ELF)], capture_output=True, text=True, check=True).stdout
    syms = {}
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3:
            syms[parts[2]] = int(parts[0], 16)
    return syms


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("script")
    ap.add_argument("--out", default=tempfile.gettempdir())
    ap.add_argument("--seed", default="3")
    ap.add_argument("--stage", type=int, default=1, help="the stage a built-in script plays (1 up)")
    ap.add_argument("--phi2", help="6502 clock in kHz (default 8000); lower it to measure CPU headroom")
    args = ap.parse_args()
    text = BUILTIN.get(args.script)
    if text is None:
        text = Path(args.script).read_text()
    syms = symbols()

    def sub(m):
        name, plus = m.group(1), m.group(2)
        if name == "out":
            return args.out
        if name == "stage":
            return str(args.stage - 1)
        if name not in syms:
            sys.exit(f"unknown symbol {name}")
        return hex(syms[name] + (int(plus) if plus else 0))

    text = re.sub(r"\{(\w+)(?:\+(\d+))?\}", sub, text)
    with tempfile.NamedTemporaryFile("w", suffix=".txt", delete=False) as f:
        f.write(text)
    cmd = [str(EMU), str(ROM), "--seed", args.seed, "--mute", "--script", f.name]
    if args.phi2:
        cmd += ["--phi2", args.phi2]
    r = subprocess.run(cmd)
    sys.exit(r.returncode)


if __name__ == "__main__":
    main()
