# MegaGammaForce

A vertical shoot-'em-up for the [Picocomputer 6502](https://picocomputer.github.io/)
in the spirit of *Radiant Silvergun*: three weapons that combine into
seven attacks, a color chain, and three stages, each ending in a boss you
take apart piece by piece.
Written in C with [llvm-mos](https://llvm-mos.org/).

<!-- rp6502
preset: llvm-mos/Release
publish: MegaGammaForce.zip
frames: 120
-->
[![MegaGammaForce](https://jasonfrowe.github.io/MegaGammaForce/MegaGammaForce/screenshot.png)](https://jasonfrowe.github.io/MegaGammaForce/MegaGammaForce/)

## How to play

### Controls

A, B and C are the three guns. Hold two at once to combine them, or use
the combination's own button.

| Action | Keyboard | Gamepad |
|---|---|---|
| Move | Arrows or WASD | D-pad or left stick |
| **A** Vulcan | Z or J | A |
| **B** Homing | X or K | B |
| **C** Spread | C or L | C or R1 |
| **A+B** Homing Plasma | V (or Z+X) | X (or A+B) |
| **A+C** Wide Shot | Z+C | Y (or A+C/R1) |
| **B+C** Homing Missiles | B (or X+C) | Z or L1 (or B+C/R1) |
| Radiant Sword | Space | R2 |
| Hyper Sword | Z+X+C | L2 |
| Start / pause | Enter or P | Start |

The ship slows down while it fires, and slows further with the sword out.
Holding all three guns fires the Vulcan; on the keyboard, pressing Z+X+C
with a full gauge also unleashes the Hyper Sword.

### Weapons

From strongest to weakest:

- **Vulcan**: fast twin bolts straight ahead. The strongest weapon, if you
  aim it.
- **Spread**: a five-way fan for crowds. Medium, and best up close where
  more of the fan connects.
- **Radiant Sword**: a blade arc above the ship that cuts anything it
  touches and **absorbs pink bullets**. Blue bullets pass through it.
- **Wide Shot**: a forward bolt plus weak shots to the sides and behind.
- **Homing Missiles**: hold it and a lock-on ring grows out from the ship.
  Everything the ring sweeps over gets a reticle and, a moment later, a
  missile of its own that chases that target and no other. Up to six locks
  a sweep, then the ring starts again. Strong against bosses with many
  parts, slow against a lone target.
- **Homing**: lock-on darts from the wingtips. They never miss, so they
  hit softly.
- **Homing Plasma**: a crackling link that latches onto the nearest target
  ahead. The weakest: it does the aiming for you.
- **Hyper Sword**: every absorbed bullet charges the SWORD gauge. When it
  reads HYPER!, press the Hyper Sword button: every bullet on screen is
  cancelled for points, and for a few seconds the blade grows, a beam
  lances up the screen, and the ship cannot be hit. Filling the gauge while
  holding the sword never sets it off.

Damage per second, against the Vulcan's 45: Spread 30 (all five orbs
hitting), Sword 22, Homing Plasma 7.5, Homing 7.5. Homing Missiles do 8 per
locked target per sweep of the ring, about a second: enough to destroy
darts, wasps, saucers and ground turrets in one hit. These are the LV1
numbers; the table is in `src/shots.c` and `src/player.c`.

### Weapon levels

As in Radiant Silvergun, each gun and the Radiant Sword gains experience
from what it destroys: a point or two for small fry, more for gunships,
gunboats and boss parts. The kill goes to the weapon that fired the shot.
At 15, 40, 80 and 150 experience the weapon goes up a level, to LV5, and
LEVEL UP! flashes in the playfield. The HUD shows the level and the
progress to the next under the weapon's name.

Every level fires faster, up to twice the LV1 rate at LV5. On top of that,
Spread fires seven orbs from LV3, Homing Missiles lock and sweep faster,
and the sword also hits harder. The Vulcan, already the strongest, gains
experience at half the rate, so the weaker weapons are the ones worth
using to level up. Levels carry over between stages and reset with a new
game.

### The chain

Enemies come in red, blue and yellow (gray ones don't count). Destroy
three of the **same color in a row** to complete a set: the chain goes up
and pays 100, 200, 500, 750, 1,000 … up 250 a step to 10,000 points. Each further kill of that
same color pays again, one step higher. Switching color starts a new set:
if the run you left was complete, the chain carries on once you get three
of the new color; if you switch in the middle of a set, the chain breaks.

    red red red    -> bonus x1
    red            -> bonus x2
    blue blue blue -> bonus x3
    red blue ...   -> broken: the lone red was an unfinished set

The three pips under CHAIN show the current run. Losing a ship loses the
chain, and the Hyper Sword charge with it.

Extra lives (EXTEND!) come at 50,000 and 100,000 points, then every 100,000 after that, up to seven ships.

### The stages

| Stage | Where | What's new | Boss |
|---|---|---|---|
| 1 | **Sky Fortress**, a fortress deck over the sea | Ground turrets | GAMMA CORE |
| 2 | **Neon Spires**, a city at night | Tall towers to dodge (flying into one is fatal), tanks on the roads, wasps | SPIRE WARDEN |
| 3 | **Iron Tide**, open ocean and jungle islands | Gunboats, island bunkers | Battleship LEVIATHAN |
| 4 | **Scorched Sands**, desert dunes, plateaus and oases | Armoured trains on a railway, a sandstorm drifting over | DUNE WYRM |
| 5 | **Frozen Strait**, pack ice and open leads | Icebergs to dodge, icebreakers in the water, snow tanks on plowed roads, blowing snow | GLACIER CRAWLER |
| 6 | **Ascent**, climbing out of the atmosphere | The ocean darkens below, the Earth's glowing limb passes under you into black space; satellites to dodge | HALO STATION |
| 7 | **Sea of Tranquility**, the Moon | Craters, lunar bases with domes and landing pads, rovers on the roads; open sky | MASS DRIVER |

On the title screen, left and right choose the stage a game starts on.
Score, lives and the chain carry from one stage to the next. Clearing
the last stage ends the game.

Stage 2's towers, stage 5's icebergs and stage 6's satellites stand as high as you fly. Flying into one is fatal, and
they stop your Vulcan, Spread, Homing darts and the Wide Shot's rear shots,
and every enemy bullet too, so they make cover as well as walls. Homing
Missiles and Homing Plasma reach over them, and so do the swords. Flying
enemies steer around them.

### The bosses

Every boss is built from parts, each with its own armor. Shooting a part
off raises the DESTROY percentage; destroying the core ends the fight.
Strip everything else first for **100% destruction** and a perfect bonus.
Every hit on a boss scores 20 points a point of damage. Destroying one
plays the victory music over the results.
Absorb big pink bullets with the sword to charge the Hyper Sword. Take
too long and the boss escapes.

- **GAMMA CORE** (90 seconds): the core, two wings with two turrets each,
  and a belly cannon that lobs big pink orbs.
- **SPIRE WARDEN** (90 seconds): three pods orbit the core, faster as they
  fall; a twin cannon sweeps needles across the screen, and the core opens
  up once two pods are gone.
- **LEVIATHAN** (two minutes): a battleship longer than the screen, sailing
  bow first. Six deck turrets, two main guns, a bridge, and two missile
  bays that open to launch fighters. Its core, the reactor, is at the
  stern, so it has to sail past before you can finish it. Destroyed parts
  leave craters in the deck.
- **DUNE WYRM** (90 seconds): a sandworm whose head winds a figure of
  eight with nine armoured segments and a tail trailing its path. Every
  segment can be shot off; the head is the core.
- **GLACIER CRAWLER** (100 seconds): a tracked fortress grinding up the
  ice at you. Twin guns, four turrets, two hatches that launch fighters,
  and an ice drill at the front that sprays pink. The cryo reactor at the
  rear is the core, and wakes up once the drill is gone.
- **HALO STATION** (100 seconds): an orbital ring station. Six turrets on
  the ring, four gun satellites orbiting outside it (faster as they fall),
  and the core in the hub, which turns to spirals when it's hurt.
- **MASS DRIVER** (100 seconds): a hovering gun platform built round a
  rail cannon. Every few seconds its muzzle glows and fires a burst of fast
  slugs straight down, so get out from under it. Capacitor pods spray pink,
  turrets and hatches cover the rest. The breech is the core, and spirals
  once the muzzle is gone.

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
| 0 | Mode 2 ground map, 16x16 4bpp tiles | Ground units: turrets, tanks, gunboats (8, 12 or 16 by stage) |
| 1 | Mode 2 sky layer: stage 2's towers and clouds, stage 3's clouds, then the battleship's hull | Everything airborne: boss, enemies, shots, ship, sword, explosions, bullets (Mode 5 custom, 4bpp) |
| 2 | Mode 1 text HUD | HUD graphics: big score digits, weapon icon, chain gems, lives ship, Hyper sign, boss portrait, stage banner |

The playfield is 192 pixels wide, close to an arcade's vertical screen.
The HUD's two side panels are opaque, so they also hide sprites leaving
the playfield. The whole stage map lives in XRAM, so scrolling is one
16-bit write a frame. Its top holds two copies of the same stretch of sea
for the boss fight to scroll over forever. The sky layer scrolls 1.5
times as fast, for parallax, and stops at its empty top before the boss
arrives. A stage can also fade part of the ground's palette toward black
as it scrolls: stage 6 darkens its ocean and clouds as you climb, while
the Earth's limb and the stars keep their light. Fill layers cost no sprite time, which is why the battleship's
hull is a tile map that the game moves around, with its guns as sprites
on top. Stage 5's crawler, stage 6's station and stage 7's mass driver are built the same way,
but from tables alone:
a scripted boss can name a `hull_t` (its map, tiles and each part's
socket), and the boss engine shows and moves the layer and writes craters
for it. The boss file is reloaded whenever a stage starts, so the craters
from the last attempt are gone.

XRAM holds what every stage shares (the ship, weapons, enemies, effects,
sound effects, HUD) and, after it, about 34 KB for the current stage, in
two parts. `ROM:stage<N>.bin` holds the stage's script (its air waves and
ground units), ground and sky tiles and maps, its palettes and any art
only it uses (stage 4's trains); it loads into `XRAM_BANK`.
`ROM:boss<N>.bin` holds the boss's art and palettes and loads into
`XRAM_BOSS`, so a boss can be swapped in mid-stage. The HUD blacks out the
playfield while they load. The title uses stage 1's files, which also
hold the logo. The build checks that the largest stage and boss fit.

Bosses 1-3 are code (`boss1.c`-`boss3.c`). Later ones are data: a
`motion_t` (entry, hover, parts that orbit, parts that trail the head like a
snake's body) and a table of `pattern_t` attacks (aimed fans, downward
fans, rings, spirals, launched enemies, each from a part, on a period,
always or once the core is angry or another part is gone), run by
`boss.c`. That costs a few hundred bytes a boss instead of a few KB,
which matters: the program and its data live in the 6502's 64 KB too.

Every sprite list is mirrored in RAM and copied to XRAM once a frame,
right after VSYNC. Bullets and shots keep 8.8 fixed point positions in a
"bullet space" where the visible playfield fits in a byte, so collision
tests are byte compares. `src/xram.h` is the whole XRAM map.

### Assets are generated

Nothing is drawn by hand yet; Python draws it all, and you can edit the
results.

| Tool | Makes |
|---|---|
| `tools/gen_art.py` | Shared sheets in `images/*.png`; each stage's sheets, maps and palettes in `images/stage<N>/`; `src/gen/assets.h`, `src/gen/stage<N>.h` (bank offsets) and `src/gen/stage<N>_data.h` (ground units, solid tiles) |
| `tools/art/` | The pixel-art kit: shapes with heights, lit from the top left, outlined. Banking frames are real rolls of the ship. `stages.py` lists what each stage's files hold and places ground units, `waves.py` has every stage's air waves; `tiles.py`, `city.py`, `ocean.py`, `desert.py`, `arctic.py`, `orbit.py`, `luna.py` draw the stages, `boss.py`, `boss2.py`, `ship.py`, `wyrm.py`, `crawler.py`, `station.py`, `driver.py` the bosses, `hud.py` the HUD graphics. |
| `tools/mkbank.py` | Run by the build: packs `images/stage<N>/` into `ROM:stage<N>.bin` and `ROM:boss<N>.bin` |
| `tools/gen_sfx.py` | OPL2 sound effects in `music/sfx.bin`, `src/gen/sfx_ids.h` |
| `tools/generate_music.py` | OPL2 music via Furnace (from RPDemo's composer): `music/fur_levels/*.fur` and `music/*.vgm` |
| `tools/gen_tables.py` | Trig tables in `src/gen/tables.c` |
| `tools/png2bin.py` | Run by the build: PNG sheets to VGA image data |

The PNG sheets are indexed 16-color images with frames side by side, so
they open directly in Aseprite. Edit and save one, rebuild, and your
change is in the game; a stage's sheets must keep their frame size and
count. Running `gen_art.py` again overwrites them.

The title, stage 1 and stage 1 boss music use style 9, **silvergun**, written for this game in
`tools/musicgen/silvergun.py`: orchestral-epic instead of RPDemo's
electronic styles, but with a rock kit underneath for drive. That means
brass and string leads playing phrases and 16th-note runs, galloping
string ostinatos, octave-jumping bass, choir pads, timpani, a bass drum and
hi-hat groove, crashes and orchestra hits, chromatic "epic minor" harmony,
and optional 6/8 time. Tempo comes in steps, because a 16th note is a whole
number of 60 Hz ticks: 6 ticks is 150 bpm, 5 is 180.

Four more styles, in `tools/musicgen/arcade.py`, give you something else
to try with `--style`:

| Style | Sound |
|---|---|
| 10 **fmrock** | Mega Drive FM rock: distorted guitar or synth brass riffs, palm-muted 16th chug, picked bass, rock organ, double-kick runs |
| 11 **eurobeat** | Synth brass hooks, a two-octave 16th broken-chord sequence, octave bass on every 8th, four-on-the-floor kick |
| 12 **chiparcade** | Square leads, a fast one-row-per-note chord arpeggio, bouncing bass, and the pad channel used as an echo of the lead |
| 13 **fusion** | Sax or synth lead, electric piano stabs, slap bass with octave pops and ghost notes, a busy funk kit |

Stage 2 uses eurobeat, stage 3 fusion and the battleship fmrock, stage 4 silvergun with Phrygian chords (`--prog 6,7`) and the wyrm fmrock, stage 5 chiparcade and the crawler eurobeat, stage 6 fusion and the station silvergun, stage 7 eurobeat and the mass driver fmrock; the stage 2 boss reuses `Boss.vgm`.
`Victory.vgm` (and its `.fur`) comes from RPDemo as it is, and plays once after a boss is destroyed.
`Gameover.vgm` is generated (style 9, `"form": "jingle"` in its mood: one phrase and a cadence home, played once).
All four are in 4/4. On a 6/8 track (the boss), they keep the same eighth-note
pulse, so they play 1.5 times as fast.

The base mix already includes the `--vol -3,3,0,0,0,0,12` balance that
RPDemo's tracks always needed, so `vol` offsets in the manifest start at 0. Track moods
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

To change only the chords, pick the two themes' progressions with
`--prog A,B`. `--list-progs` prints the banks, such as the "royal road"
IV-V-iii-vi. The melody, rhythms and instruments of the seed stay the same,
and the choice is recorded in the manifest:

```sh
.venv/bin/python tools/generate_music.py --list-progs            # every style's banks
.venv/bin/python tools/generate_music.py --list-progs --style 11 # one style's
.venv/bin/python tools/generate_music.py --track title --from-manifest --prog 6,0 --export-vgm
```

#### Trying out new music

The `.vgm` files in `music/` are what the game plays. The build packs
`music/Title.vgm`, `Stage1.vgm` and `Boss.vgm` into the ROM as they are,
so nothing else needs converting. The `.fur` files are only for editing
in Furnace.

Add `--scratch` to try ideas without touching the game's tracks. Takes go
to `music/scratch/` (which is gitignored), are named by track, style and
seed, and are not recorded in the manifest:

```sh
# Each style once at the Stage 1 mood, to hear the differences.
for s in 9 10 11 12 13; do
  .venv/bin/python tools/generate_music.py --track stage1 --style $s --scratch --export-vgm
done
# Then vary one you like: the same seed with other chords, or another tempo.
.venv/bin/python tools/generate_music.py --track stage1 --style 11 --seed 42 --prog 2,3 --scratch --export-vgm
.venv/bin/python tools/generate_music.py --track stage1 --style 11 --seed 42 --bpm 150 --scratch --export-vgm
```

Open a take's `.fur` in Furnace to listen and edit it. To hear it in the
game, either:

- **Keep it:** rerun the take's command without `--scratch`. Every run prints
  its command. That writes `music/Stage1.vgm` and records the take in the
  manifest.
- **Use your Furnace edits:** in Furnace, choose File > Export > VGM and save over
  `music/Stage1.vgm`. The songs this tool writes are already OPL2 only. You can also
  export without the GUI:
  `furnace -subsong 0 -loops 1 -vgmout music/Stage1.vgm music/scratch/<take>.fur`.

Then rebuild. The exported VGM must be plain OPL2 on channels 1-7: the
game's player doesn't understand OPL3, and channels 8-9 are kept for sound
effects.

`--bpm` takes any number, but the speed is a whole number of 60 Hz ticks
per 16th note, so the tempo lands on 900/n: 180, 150, 128.6, 112.5, 100.

### Testing in the emulator

`tools/playtest.py` runs emulator scripts against the build, filling in
`{symbol}` addresses from the ELF. `boss` warps to the boss and fights it
invulnerable; `soak` plays the stage holding fire. Both print
`lag_frames`, the count of frames where the game logic overran, which
stays at 0. `--stage N` picks the stage (scripts of your own can poke
`first_stage` on the title screen), and `--phi2` slows the 6502 to
measure headroom: every stage keeps up down to 7 MHz.

```sh
.venv/bin/python tools/playtest.py boss --stage 3 --out /tmp
```
