# MegaGammaForce

A vertical shoot-'em-up for the [Picocomputer 6502](https://picocomputer.github.io/)
in the spirit of *Radiant Silvergun*: three weapons that combine into
seven attacks, a color chain, and a boss you take apart piece by piece.
Written in C with [llvm-mos](https://llvm-mos.org/).

<!-- rp6502
preset: llvm-mos/Release
publish: MegaGammaForce.zip
frames: 120
-->
[![MegaGammaForce](https://jasonfrowe.github.io/MegaGammaForce/MegaGammaForce/screenshot.png)](https://jasonfrowe.github.io/MegaGammaForce/MegaGammaForce/)

## How to play

### Controls

| Action | Keyboard | Gamepad |
|---|---|---|
| Move | Arrows or WASD | D-pad or left stick |
| **A** Vulcan | Z or J | X |
| **B** Homing | X or K | A |
| **C** Spread | C or L | B |
| **A+B** Back Wide | V (or hold Z+X) | L1 |
| **B+C** Homing Plasma | B (or hold X+C) | R1 |
| **A+C** Radiant Sword | N or Space (or hold Z+C) | Y |
| **A+B+C** Hyper Sword | M (or hold all three) | L2 / R2 |
| Start / pause | Enter or P | Start |

The ship slows down while it fires, and slows further with the sword out.

### Weapons

From strongest to weakest:

- **Vulcan**: fast twin bolts straight ahead. The strongest weapon, if you
  aim it.
- **Spread**: a five-way fan for crowds. Medium, and best up close where
  more of the fan connects.
- **Radiant Sword**: a blade arc above the ship that cuts anything it
  touches and **absorbs pink bullets**. Blue bullets pass through it.
- **Back Wide**: a forward bolt plus weak shots to the sides and behind.
- **Homing**: lock-on darts from the wingtips. They never miss, so they
  hit softly.
- **Homing Plasma**: a crackling link that latches onto the nearest target
  ahead. The weakest: it does the aiming for you.
- **Hyper Sword**: every absorbed bullet charges the SWORD gauge. When it
  reads HYPER!, hold all three buttons: every bullet on screen is cancelled
  for points, and for a few seconds the blade grows, a beam lances up the
  screen, and the ship cannot be hit.

Damage per second, against the Vulcan's 45: Spread 30 (all five orbs
hitting), Sword 22, Homing 10, Homing Plasma 7.5. The table is in
`src/shots.c` and `src/player.c`.

### The chain

Enemies come in red, blue and yellow (gray ones don't count). Destroy
three of the **same color in a row** to complete a set: the chain goes up
and pays 100, 200, 400 … up to 10,000 points per set. Hitting a different
color in the middle of a set breaks the chain. Once a set is complete, the
next set can be any color. The three pips under CHAIN show the set so far.

### The boss

GAMMA CORE is built from eight parts: the core, two wings, four wing
turrets and a belly cannon. Each part has its own armor. Shooting a part
off raises the DESTROY percentage; destroying the core ends the fight.
Strip everything else first for **100% destruction** and a perfect bonus.
Absorb the cannon's big pink orbs with the sword to charge the Hyper Sword.
If you take longer than 90 seconds, the boss escapes.

## Building

You need the [RP6502 SDK](https://picocomputer.github.io/sdk.html) tools
(CMake 3.21+, Python 3, llvm-mos). Then:

```sh
cmake --preset llvm-mos/Release
cmake --build --preset llvm-mos/Release
./tools/rp6502-emu build/llvm-mos/release/MegaGammaForce.rp6502
```

The build itself needs only Python's standard library. Regenerating the
art needs Pillow and numpy in the project's virtual environment:

```sh
python3 -m venv .venv
.venv/bin/pip install -r requirements.txt
```

## How it's made

### Screen layout

The 320x240 canvas uses all three planes:

| Plane | Fill layer | Sprite layer |
|---|---|---|
| 0 | Mode 2 stage map, 16x16 4bpp tiles | Ground turrets (Mode 5 custom) |
| 1 | — | Everything airborne: boss, enemies, shots, ship, sword, explosions, bullets (Mode 5 custom, 4bpp) |
| 2 | Mode 1 text HUD | — |

The playfield is 192 pixels wide, close to an arcade's vertical screen.
The HUD's two side panels are opaque, so they also hide sprites leaving
the playfield. The whole stage map lives in XRAM, so scrolling is one
16-bit write a frame. Its top holds two copies of the same stretch of sea
for the boss fight to scroll over forever.

Every sprite list is mirrored in RAM and copied to XRAM once a frame,
right after VSYNC. Bullets and shots keep 8.8 fixed point positions in a
"bullet space" where the visible playfield fits in a byte, so collision
tests are byte compares. `src/xram.h` is the whole XRAM map.

### Assets are generated

Nothing is drawn by hand yet; Python draws it all, and you can edit the
results.

| Tool | Makes |
|---|---|
| `tools/gen_art.py` | `images/*.png` sprite and tile sheets, `images/palettes.bin`, the stage map and turret positions, `src/gen/assets.h` |
| `tools/art/` | The pixel-art kit: shapes with heights, lit from the top left, outlined. Banking frames are real rolls of the ship. |
| `tools/gen_sfx.py` | OPL2 sound effects in `music/sfx.bin`, `src/gen/sfx_ids.h` |
| `tools/generate_music.py` | OPL2 music via Furnace (from RPDemo's composer): `music/fur_levels/*.fur` and `music/*.vgm` |
| `tools/gen_tables.py` | Trig tables in `src/gen/tables.c` |
| `tools/png2bin.py` | Run by the build: PNG sheets to VGA image data |

The PNG sheets are indexed 16-color images with frames side by side, so
they open directly in Aseprite. Edit and save one, rebuild, and your
change is in the game. Running `gen_art.py` again overwrites them.

The music uses style 9, **silvergun**, written for this game in
`tools/musicgen/silvergun.py`: orchestral-epic instead of RPDemo's
electronic styles. That means brass and string leads playing phrases,
string ostinatos, choir pads, pitched timpani, a march snare and orchestra
hits, chromatic "epic minor" harmony, and optional 6/8 time. Track moods
(key, tempo, meter, intensity) are in `tools/musicgen/tracks.py`.

Every generated track is recorded in `music/music_manifest.json`: its
seed, style, per-role volume offsets and patches, mood, and the exact
command. To tweak a track, edit its `vol` or `patch` there (roles in
`lead,arp,bass,pad,kick,snare,hat` order; patches are hex indices into
RPTracker's bank) and rebuild it without re-rolling the composition:

```sh
.venv/bin/python tools/generate_music.py --track boss --from-manifest --export-vgm
.venv/bin/python tools/generate_music.py --track all --from-manifest --export-vgm
```

`--vol`/`--patch` on that command line override the recorded values for
one run. A new `--seed` re-rolls the composition. Then rebuild the game.

### Testing in the emulator

`tools/playtest.py` runs emulator scripts against the build, filling in
`{symbol}` addresses from the ELF. `boss` warps to the boss and fights it
invulnerable; `soak` plays the stage holding fire. Both print
`lag_frames`, the count of frames where the game logic overran, which
stays at 0.

```sh
.venv/bin/python tools/playtest.py boss --out /tmp
```
