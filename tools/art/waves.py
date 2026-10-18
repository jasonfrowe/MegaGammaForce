"""Each stage's air waves, packed into its ROM file by gen_art.py.

A wave is a group of enemies:
  (at, type, color, x, dx, count, gap, param)
  at     scroll distance in ground pixels when the group starts
  type   enemy type (ET_ name in enemies.h)
  color  chain color; gray never chains. Colors come in threes so every
         formation can complete a chain set.
  x      playfield x of the first one; each next one is dx further across
  count  how many, gap frames apart
  param  the type's own setting: where a dart dives or a drone stops (y),
         a saucer's or wasp's weave (pixels), a tank's speed (bit 7 = left)

Ground units come from the art (pads, roads, open water) in stages.py.
"""

# Stage 1, Sky Fortress.
S1_WAVES = [
    (120, "ET_DART", "COL_RED", 40, 0, 3, 16, 110),
    (220, "ET_DART", "COL_BLUE", 150, 0, 3, 16, 110),
    (330, "ET_SAUCER", "COL_YELLOW", 60, 0, 3, 22, 40),
    (430, "ET_SAUCER", "COL_RED", 130, 0, 3, 22, 40),
    (540, "ET_DART", "COL_BLUE", 24, 24, 3, 12, 90),
    (560, "ET_DART", "COL_BLUE", 168, -24, 3, 12, 90),
    (680, "ET_DRONE", "COL_YELLOW", 50, 0, 1, 1, 70),
    (700, "ET_DRONE", "COL_YELLOW", 142, 0, 1, 1, 70),
    (730, "ET_DRONE", "COL_YELLOW", 96, 0, 1, 1, 50),
    (860, "ET_DART", "COL_RED", 96, 0, 3, 10, 120),
    (900, "ET_SAUCER", "COL_BLUE", 50, 0, 3, 20, 30),
    (960, "ET_SAUCER", "COL_BLUE", 140, 0, 3, 20, 30),
    (1060, "ET_GUNSHIP", "COL_GRAY", 96, 0, 1, 1, 60),
    (1250, "ET_DART", "COL_YELLOW", 30, 20, 3, 12, 100),
    (1300, "ET_DART", "COL_RED", 162, -20, 3, 12, 100),
    (1400, "ET_DRONE", "COL_BLUE", 40, 56, 3, 30, 60),
    (1550, "ET_SAUCER", "COL_RED", 96, 0, 3, 18, 60),
    (1650, "ET_DART", "COL_BLUE", 30, 16, 3, 10, 80),
    (1700, "ET_DART", "COL_BLUE", 162, -16, 3, 10, 80),
    (1800, "ET_DRONE", "COL_RED", 48, 0, 1, 1, 80),
    (1820, "ET_DRONE", "COL_RED", 144, 0, 1, 1, 80),
    (1840, "ET_DRONE", "COL_RED", 96, 0, 1, 1, 60),
    (1950, "ET_GUNSHIP", "COL_GRAY", 60, 0, 1, 1, 50),
    (1990, "ET_GUNSHIP", "COL_GRAY", 132, 0, 1, 1, 80),
    (2250, "ET_SAUCER", "COL_YELLOW", 60, 0, 3, 18, 40),
    (2280, "ET_SAUCER", "COL_YELLOW", 130, 0, 3, 18, 40),
    (2400, "ET_DART", "COL_RED", 20, 30, 6, 8, 100),
    (2500, "ET_DART", "COL_BLUE", 172, -30, 6, 8, 100),
    (2600, "ET_DRONE", "COL_YELLOW", 40, 56, 3, 25, 70),
    (2750, "ET_DART", "COL_RED", 96, 0, 3, 10, 130),
    (2780, "ET_DART", "COL_BLUE", 60, 0, 3, 10, 130),
    (2810, "ET_DART", "COL_YELLOW", 132, 0, 3, 10, 130),
]

# Stage 2, Neon Spires: lighter in the air, since the towers take room.
S2_WAVES = [
    (140, "ET_WASP", "COL_RED", 50, 0, 3, 16, 30),
    (200, "ET_WASP", "COL_RED", 140, 0, 3, 16, 30),
    (260, "ET_DART", "COL_BLUE", 30, 26, 3, 12, 100),
    (400, "ET_DART", "COL_BLUE", 162, -26, 3, 12, 100),
    (540, "ET_SAUCER", "COL_YELLOW", 96, 0, 3, 22, 50),
    (700, "ET_WASP", "COL_BLUE", 40, 20, 3, 12, 24),
    (740, "ET_WASP", "COL_YELLOW", 150, -20, 3, 12, 24),
    (860, "ET_DRONE", "COL_RED", 60, 36, 3, 30, 70),
    (1020, "ET_GUNSHIP", "COL_GRAY", 96, 0, 1, 1, 60),
    (1220, "ET_WASP", "COL_YELLOW", 30, 0, 3, 14, 20),
    (1250, "ET_WASP", "COL_YELLOW", 162, 0, 3, 14, 20),
    (1390, "ET_SAUCER", "COL_BLUE", 50, 0, 3, 20, 40),
    (1420, "ET_SAUCER", "COL_RED", 140, 0, 3, 20, 40),
    (1560, "ET_DART", "COL_YELLOW", 20, 30, 3, 10, 110),
    (1720, "ET_DRONE", "COL_BLUE", 40, 56, 3, 30, 60),
    (1900, "ET_WASP", "COL_RED", 96, 0, 3, 12, 40),
    (2060, "ET_GUNSHIP", "COL_GRAY", 96, 0, 1, 1, 60),
    (2260, "ET_DART", "COL_BLUE", 172, -30, 3, 10, 100),
    (2300, "ET_WASP", "COL_BLUE", 30, 14, 3, 10, 30),
    (2400, "ET_SAUCER", "COL_YELLOW", 60, 0, 3, 20, 40),
    (2570, "ET_WASP", "COL_BLUE", 40, 0, 3, 10, 24),
    (2610, "ET_WASP", "COL_YELLOW", 152, 0, 3, 10, 24),
    (2770, "ET_DART", "COL_RED", 96, 0, 3, 12, 130),
]

# Stage 3, Iron Tide: everything at once over the sea.
S3_WAVES = [
    (120, "ET_DART", "COL_YELLOW", 30, 22, 3, 12, 100),
    (180, "ET_DART", "COL_YELLOW", 162, -22, 3, 12, 100),
    (300, "ET_WASP", "COL_RED", 60, 0, 3, 12, 30),
    (340, "ET_WASP", "COL_RED", 132, 0, 3, 12, 30),
    (460, "ET_SAUCER", "COL_BLUE", 40, 0, 3, 18, 36),
    (490, "ET_SAUCER", "COL_BLUE", 152, 0, 3, 18, 36),
    (620, "ET_GUNSHIP", "COL_GRAY", 96, 0, 1, 1, 60),
    (760, "ET_DRONE", "COL_YELLOW", 50, 46, 3, 24, 70),
    (880, "ET_WASP", "COL_BLUE", 20, 26, 6, 9, 30),
    (1000, "ET_DART", "COL_RED", 172, -26, 6, 8, 110),
    (1120, "ET_SAUCER", "COL_YELLOW", 96, 0, 3, 16, 60),
    (1150, "ET_WASP", "COL_YELLOW", 40, 0, 3, 10, 20),
    (1180, "ET_WASP", "COL_YELLOW", 152, 0, 3, 10, 20),
    (1320, "ET_GUNSHIP", "COL_GRAY", 50, 0, 1, 1, 50),
    (1340, "ET_GUNSHIP", "COL_GRAY", 142, 0, 1, 1, 80),
    (1500, "ET_DRONE", "COL_RED", 60, 0, 1, 1, 70),
    (1520, "ET_DRONE", "COL_RED", 132, 0, 1, 1, 70),
    (1540, "ET_DRONE", "COL_RED", 96, 0, 1, 1, 50),
    (1660, "ET_DART", "COL_BLUE", 30, 18, 6, 8, 120),
    (1780, "ET_WASP", "COL_RED", 96, 0, 6, 8, 44),
    (1900, "ET_SAUCER", "COL_BLUE", 50, 0, 3, 16, 40),
    (1930, "ET_SAUCER", "COL_BLUE", 140, 0, 3, 16, 40),
    (2060, "ET_DRONE", "COL_YELLOW", 40, 56, 3, 22, 60),
    (2200, "ET_GUNSHIP", "COL_GRAY", 96, 0, 1, 1, 60),
    (2340, "ET_WASP", "COL_RED", 30, 0, 3, 8, 24),
    (2350, "ET_WASP", "COL_BLUE", 96, 0, 3, 8, 24),
    (2360, "ET_WASP", "COL_YELLOW", 162, 0, 3, 8, 24),
    (2500, "ET_DART", "COL_YELLOW", 20, 30, 6, 7, 110),
    (2600, "ET_DART", "COL_RED", 172, -30, 6, 7, 110),
    (2720, "ET_GUNSHIP", "COL_GRAY", 60, 0, 1, 1, 50),
    (2750, "ET_GUNSHIP", "COL_GRAY", 132, 0, 1, 1, 70),
]

# Stage 4, Scorched Sands: trains on the rails below (stages.py), saucers
# and drones in the dust, wasps in swarms.
S4_WAVES = [
    (140, "ET_DART", "COL_YELLOW", 30, 24, 3, 12, 100),
    (240, "ET_DART", "COL_YELLOW", 162, -24, 3, 12, 100),
    (360, "ET_SAUCER", "COL_RED", 60, 0, 3, 20, 44),
    (420, "ET_SAUCER", "COL_BLUE", 132, 0, 3, 20, 44),
    (560, "ET_WASP", "COL_RED", 40, 18, 6, 9, 28),
    (760, "ET_DRONE", "COL_BLUE", 50, 46, 3, 26, 64),
    (900, "ET_GUNSHIP", "COL_GRAY", 96, 0, 1, 1, 60),
    (1040, "ET_DART", "COL_RED", 20, 30, 6, 8, 110),
    (1200, "ET_WASP", "COL_YELLOW", 96, 0, 3, 12, 50),
    (1240, "ET_WASP", "COL_YELLOW", 40, 0, 3, 12, 20),
    (1380, "ET_SAUCER", "COL_BLUE", 96, 0, 3, 18, 60),
    (1520, "ET_DRONE", "COL_RED", 60, 36, 3, 30, 70),
    (1700, "ET_GUNSHIP", "COL_GRAY", 60, 0, 1, 1, 50),
    (1740, "ET_GUNSHIP", "COL_GRAY", 132, 0, 1, 1, 80),
    (1900, "ET_DART", "COL_BLUE", 172, -30, 6, 8, 100),
    (2040, "ET_WASP", "COL_RED", 30, 14, 3, 10, 30),
    (2070, "ET_WASP", "COL_RED", 160, -14, 3, 10, 30),
    (2200, "ET_SAUCER", "COL_YELLOW", 60, 0, 3, 18, 40),
    (2230, "ET_SAUCER", "COL_YELLOW", 130, 0, 3, 18, 40),
    (2380, "ET_DRONE", "COL_BLUE", 40, 56, 3, 22, 60),
    (2540, "ET_WASP", "COL_RED", 40, 0, 3, 8, 22),
    (2550, "ET_WASP", "COL_BLUE", 96, 0, 3, 8, 22),
    (2560, "ET_WASP", "COL_YELLOW", 152, 0, 3, 8, 22),
    (2720, "ET_DART", "COL_RED", 96, 0, 3, 10, 130),
    (2770, "ET_GUNSHIP", "COL_GRAY", 96, 0, 1, 1, 60),
]

# Stage 5, Frozen Strait: icebergs take room, so the air is lighter than
# stage 4's; tanks on the roads and icebreakers in the leads below.
S5_WAVES = [
    (140, "ET_SAUCER", "COL_BLUE", 60, 0, 3, 20, 40),
    (220, "ET_SAUCER", "COL_BLUE", 132, 0, 3, 20, 40),
    (360, "ET_DART", "COL_RED", 30, 24, 3, 12, 100),
    (480, "ET_DART", "COL_RED", 162, -24, 3, 12, 100),
    (620, "ET_WASP", "COL_YELLOW", 96, 0, 3, 14, 40),
    (780, "ET_DRONE", "COL_BLUE", 50, 46, 3, 26, 64),
    (940, "ET_GUNSHIP", "COL_GRAY", 96, 0, 1, 1, 60),
    (1100, "ET_WASP", "COL_RED", 30, 14, 3, 10, 30),
    (1140, "ET_WASP", "COL_RED", 160, -14, 3, 10, 30),
    (1300, "ET_DART", "COL_YELLOW", 20, 30, 6, 8, 110),
    (1460, "ET_SAUCER", "COL_RED", 96, 0, 3, 18, 60),
    (1620, "ET_DRONE", "COL_YELLOW", 60, 36, 3, 30, 70),
    (1780, "ET_GUNSHIP", "COL_GRAY", 60, 0, 1, 1, 50),
    (1820, "ET_GUNSHIP", "COL_GRAY", 132, 0, 1, 1, 80),
    (1980, "ET_DART", "COL_BLUE", 172, -30, 6, 8, 100),
    (2140, "ET_WASP", "COL_BLUE", 40, 0, 3, 10, 24),
    (2170, "ET_WASP", "COL_YELLOW", 152, 0, 3, 10, 24),
    (2320, "ET_SAUCER", "COL_YELLOW", 60, 0, 3, 18, 40),
    (2350, "ET_SAUCER", "COL_RED", 130, 0, 3, 18, 40),
    (2500, "ET_DRONE", "COL_RED", 40, 56, 3, 22, 60),
    (2660, "ET_DART", "COL_BLUE", 96, 0, 3, 10, 130),
    (2780, "ET_GUNSHIP", "COL_GRAY", 96, 0, 1, 1, 60),
]

# Stage 6, Ascent: nothing on the ground any more, so the air is busier,
# with satellites to dodge higher up.
S6_WAVES = [
    (120, "ET_DART", "COL_BLUE", 30, 24, 3, 12, 100),
    (180, "ET_DART", "COL_BLUE", 162, -24, 3, 12, 100),
    (300, "ET_SAUCER", "COL_RED", 50, 0, 3, 18, 40),
    (330, "ET_SAUCER", "COL_RED", 142, 0, 3, 18, 40),
    (460, "ET_WASP", "COL_YELLOW", 40, 18, 6, 9, 28),
    (600, "ET_DRONE", "COL_BLUE", 50, 46, 3, 24, 64),
    (720, "ET_GUNSHIP", "COL_GRAY", 60, 0, 1, 1, 50),
    (750, "ET_GUNSHIP", "COL_GRAY", 132, 0, 1, 1, 80),
    (880, "ET_DART", "COL_RED", 20, 30, 6, 8, 110),
    (1000, "ET_WASP", "COL_BLUE", 96, 0, 6, 8, 44),
    (1120, "ET_SAUCER", "COL_YELLOW", 96, 0, 3, 16, 60),
    (1240, "ET_DRONE", "COL_RED", 60, 0, 1, 1, 70),
    (1260, "ET_DRONE", "COL_RED", 132, 0, 1, 1, 70),
    (1280, "ET_DRONE", "COL_RED", 96, 0, 1, 1, 50),
    (1420, "ET_DART", "COL_YELLOW", 30, 18, 6, 8, 120),
    (1560, "ET_GUNSHIP", "COL_GRAY", 96, 0, 1, 1, 60),
    (1700, "ET_WASP", "COL_RED", 40, 0, 3, 10, 22),
    (1730, "ET_WASP", "COL_BLUE", 152, 0, 3, 10, 22),
    (1880, "ET_SAUCER", "COL_BLUE", 50, 0, 3, 16, 40),
    (1910, "ET_SAUCER", "COL_YELLOW", 140, 0, 3, 16, 40),
    (2060, "ET_DRONE", "COL_YELLOW", 40, 56, 3, 22, 60),
    (2200, "ET_DART", "COL_RED", 172, -30, 6, 7, 110),
    (2340, "ET_GUNSHIP", "COL_GRAY", 60, 0, 1, 1, 50),
    (2370, "ET_GUNSHIP", "COL_GRAY", 132, 0, 1, 1, 70),
    (2500, "ET_WASP", "COL_RED", 30, 0, 3, 8, 24),
    (2510, "ET_WASP", "COL_BLUE", 96, 0, 3, 8, 24),
    (2520, "ET_WASP", "COL_YELLOW", 162, 0, 3, 8, 24),
    (2660, "ET_DART", "COL_BLUE", 96, 0, 3, 10, 130),
    (2780, "ET_SAUCER", "COL_RED", 96, 0, 3, 14, 50),
]
