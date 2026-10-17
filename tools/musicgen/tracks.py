"""Registry of the game's music slots and the mood each one should evoke.

Mapping confirmed by reading MegaGammaForce's source directly:
  - src/music.c              default/title track
  - src/gameplay.c           track_for_level() -- one distinct track per
                              level 1-7; level 8+ falls back to Level_07
                              (there's no higher-level track, so the last
                              one just keeps playing)
  - src/gameplay.c           gameplay_reset_to_title_scene() -> title track
  - src/gameplay_boss.c      BOSS_STAGE_MUSIC_TRACK (src/constants.h)
  - src/level_bonus.c        bonus round
  - src/gameplay_game_over.c game over (loss) / victory (win)

`resource` is both the ROM asset name game code opens via
"ROM:<resource>.vgm" (see CMakeLists.txt's
rp6502_asset(... <resource>.vgm music/<resource>.vgm) lines)
and this tool's output filename stem (music/<resource>.vgm,
music/fur_levels/<resource>.fur) -- so it doubles as the single source of
truth tying the generator, the build, and the game's `music_set_track()`
calls together. Previously these were opaque "RESOURCE.NNN" ids (a
leftover of an older, smaller track count that reused one track across two
levels, e.g. level 1 & 5 shared RESOURCE.005) -- renamed to these
self-documenting names once every level got its own distinct track.
"""

from __future__ import annotations

from dataclasses import dataclass


@dataclass
class MoodPreset:
    """Musical parameters for a track's mood. Semitone 0 = C.

    `intensity` (0.0-1.0) places the track on the game's arc and drives
    compose.py's choice of chord progressions (title/early levels stay
    simple and diatonic; boss fights and late levels pull from a pool of
    more harmonically restless progressions) and whether the contrasting
    B-theme/bridge modulates to a different key.
    """

    key_root: int
    scale: str
    bpm: float
    intensity: float
    target_seconds: float = 170.0
    lead_octave: int = 5
    arp_octave: int = 4
    bass_octave: int = 2
    use_drums: bool = True
    meter: str = "4/4"  # "4/4", "6/8" or "3/4"; only style 9 (silvergun) reads it


@dataclass
class TrackSpec:
    resource: str  # e.g. "Level_01" -- ROM asset name and output filename stem
    description: str
    aliases: list
    mood: MoodPreset


TRACKS: list = [
    TrackSpec(
        resource="Title",
        description="Title / attract screen",
        aliases=["title"],
        # Slow heroic 6/8; bpm counts dotted quarters.
        mood=MoodPreset(key_root=2, scale="minor", bpm=60, intensity=0.30, target_seconds=100.0, lead_octave=5,
                         meter="6/8"),
    ),
    TrackSpec(
        resource="Stage1",
        description="Stage 1, Sky Fortress",
        aliases=["stage1", "level1"],
        mood=MoodPreset(key_root=9, scale="minor", bpm=150, intensity=0.60, target_seconds=150.0, lead_octave=5),
    ),
    TrackSpec(
        resource="Boss",
        description="Boss battle, GAMMA CORE",
        aliases=["boss"],
        # Galloping 6/8; bpm counts dotted quarters.
        mood=MoodPreset(key_root=4, scale="minor", bpm=96, intensity=0.95, target_seconds=110.0,
                         lead_octave=5, arp_octave=4, meter="6/8"),
    ),
]

_ALIAS_INDEX = {}
for _t in TRACKS:
    _ALIAS_INDEX[_t.resource] = _t
    for _a in _t.aliases:
        _ALIAS_INDEX[_a] = _t


def resolve(name: str) -> TrackSpec:
    key = name.strip().lower()
    for candidate_key, spec in _ALIAS_INDEX.items():
        if candidate_key.lower() == key:
            return spec
    raise KeyError(f"unknown track '{name}'. Known: {sorted(set(_ALIAS_INDEX.keys()))}")
