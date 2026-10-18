"""What each stage's bank holds, for gen_art.py.

A stage is a dict:
  n          stage number, 1 up
  palettes   {slot: palette} for palettes.STAGE_SLOTS (missing ones are blank)
  items      bank contents in order: {"name", "frames", "pal", "idx"} for a
             sheet, or {"name", "data"} for raw bytes (a tile map)
  defines    extra #defines for src/gen/stage<N>.h, prefixed S<N>_
  data_h     C for src/gen/stage<N>_data.h: ground units, solid tiles
  previews   {name: (image, palette)} written with --preview
"""

import numpy as np

import boss
import boss2
import city
import logo
import ocean
import palettes
import ship
import tiles

COLOR = ["COL_RED", "COL_BLUE", "COL_YELLOW", "COL_GRAY"]


def ground_table(n, units):
    """units: (map y from the bottom, playfield x, C type, C color, param), any order."""
    units = sorted(units)
    out = [f"#define S{n}_GROUND_N {len(units)}",
           f"static const ground_t s{n}_ground[S{n}_GROUND_N] = {{"]
    out += [f"    {{{y}, {x}, {t}, {c}, {p}}}," for y, x, t, c, p in units]
    out += ["};", ""]
    return "\n".join(out)


def solid_table(n, name, solid_ids, count):
    """A bit per tile id: is it solid?"""
    bits = [0] * ((count + 7) // 8)
    for i in solid_ids:
        bits[i >> 3] |= 1 << (i & 7)
    return (f"static const uint8_t s{n}_{name}_solid[{len(bits)}] = {{"
            + ", ".join(f"0x{b:02X}" for b in bits) + "};\n")


def map_defines(prefix, m, loop_rows=0):
    return {f"{prefix}_ROWS": m.shape[0], f"{prefix}_COLS": m.shape[1], f"{prefix}_LOOP_ROWS": loop_rows}


def stage1():
    tl, ids, stage_map, hatches = tiles.stage1()
    pal = palettes.STAGE1
    # Ground turrets sit on the hatches. Y is the turret center measured up from
    # the bottom of the map; the stage spawns it as Y scrolls onto the screen.
    g = sorted(((r + 1) * 16, (c + 1) * 16) for r, c in hatches)
    units = [(y, x, "ET_TURRET", COLOR[i % 3], 0) for i, (y, x) in enumerate(g)]
    defines = map_defines("MAP", stage_map, tiles.LOOP_ROWS)
    defines.update({"TILES_N": len(tl), "SKY_ROWS": 0})
    return {
        "n": 1,
        "palettes": pal,
        "items": [
            {"name": "tiles", "frames": tl, "pal": pal["TILES"]},
            {"name": "map", "data": stage_map.astype(np.uint8).tobytes()},
            {"name": "logo", "frames": logo.logo(), "pal": pal["BOSS"]},
            {"name": "boss_core", "frames": boss.core()},
            {"name": "boss_wing", "frames": boss.wing()},
            {"name": "boss_cannon", "frames": boss.cannon()},
            {"name": "boss_turret", "frames": boss.turret()},
        ],
        "defines": defines,
        "data_h": ground_table(1, units),
    }


def tilemap_image(tiles_list, m):
    """The whole map as one image, for previews."""
    rows = [np.concatenate([tiles_list[t] for t in row], axis=1) for row in m]
    return np.concatenate(rows, axis=0)


# Where the waves end on stages 2 and 3 (stage.c), in ground pixels: the
# sky must have reached its empty top rows by then.
WAVES_END = 2950


def stage2():
    gt, gmap, pads, roads, body_rows = city.stage2_ground()
    sky_speed = 6  # quarters: the sky scrolls 1.5 times as fast as the ground
    sky_rows = (WAVES_END * sky_speed // 4 + 240) // 16 + 1
    st, smap, solid, towers = city.stage2_sky(sky_rows)
    pal = {"TILES": city.CITY, "SKY": city.SKY, "BOSS": palettes.BOSS2, "BOSS_WRECK": palettes.BOSS2_WRECK}
    units = []
    for i, (r, c) in enumerate(sorted(pads)):
        y = (r + 1) * 16
        if 300 < y < WAVES_END:
            units.append((y, (c + 1) * 16, "ET_TURRET", COLOR[i % 3], 0))
    # Tanks on every other road, alternating directions, colors in threes.
    k = 0
    for r in roads:
        y = r * 16 + 8
        if y < 500 or y > WAVES_END - 100 or (r // city.BLOCK) % 2:
            continue
        left = k % 2 == 1
        units.append((y, 200 if left else 0, "ET_TANK", COLOR[(k // 3) % 3], (0x80 if left else 0) | 10))
        k += 1
    defines = map_defines("MAP", gmap, city.LOOP_ROWS)
    defines.update({"TILES_N": len(gt), "SKY_ROWS": smap.shape[0], "SKY_SPEED": sky_speed, "SKY_TILES_N": len(st)})
    return {
        "n": 2,
        "palettes": pal,
        "items": [
            {"name": "tiles", "frames": gt, "pal": city.CITY},
            {"name": "map", "data": gmap.astype(np.uint8).tobytes()},
            {"name": "sky_tiles", "frames": st, "pal": city.SKY},
            {"name": "sky_map", "data": smap.astype(np.uint8).tobytes()},
            {"name": "boss_core", "frames": boss2.core(), "pal": palettes.BOSS2},
            {"name": "boss_pod", "frames": boss2.pod(), "pal": palettes.BOSS2},
            {"name": "boss_cannon", "frames": boss2.cannon(), "pal": palettes.BOSS2},
            {"name": "boss_turret", "frames": boss.turret(), "pal": palettes.BOSS2},
        ],
        "defines": defines,
        "data_h": ground_table(2, units) + solid_table(2, "sky", solid, len(st)),
        "previews": {"ground": (tilemap_image(gt, gmap), city.CITY), "sky": (tilemap_image(st, smap), city.SKY)},
    }


def stage3():
    gt, gmap, bunkers, water = ocean.stage3_ground()
    sky_speed = 6
    sky_rows = (WAVES_END * sky_speed // 4 + 240) // 16 + 1
    st, smap = ocean.stage3_sky(sky_rows)
    ht, hids = ship.hull_tileset()
    hmap = ship.hull_map(hids)
    pal = {"TILES": ocean.OCEAN, "SKY": ocean.SKY, "LAYER": ship.HULL, "BOSS": ship.BOSS3,
           "BOSS_WRECK": ship.BOSS3_WRECK}
    units = []
    for i, (r, c) in enumerate(sorted(bunkers)):
        y = (r + 1) * 16
        if 300 < y < WAVES_END:
            units.append((y, (c + 1) * 16, "ET_TURRET", COLOR[i % 3], 0))
    boats = [(r, c) for r, c in sorted(water) if 600 < r * 16 < WAVES_END - 200][::2]
    for k, (r, c) in enumerate(boats):
        units.append((r * 16 + 8, c * 16 + 8, "ET_GUNBOAT", COLOR[(k // 3) % 3], 0))
    defines = map_defines("MAP", gmap, ocean.LOOP_ROWS)
    defines.update({"TILES_N": len(gt), "SKY_ROWS": smap.shape[0], "SKY_SPEED": sky_speed, "SKY_TILES_N": len(st),
                    "HULL_W": ship.W, "HULL_H": ship.H, "HULL_TILES_N": len(ht)})
    # Each part's place on the hull: tile column/row of its top left, and
    # its center relative to the hull's center, in pixels.
    for name, (c, r, w, h) in ship.PARTS.items():
        defines[f"{name}_COL"] = c
        defines[f"{name}_ROW"] = r
        defines[f"{name}_OX"] = c * 16 + w * 8 - ship.W * 8
        defines[f"{name}_OY"] = r * 16 + h * 8 - ship.H * 8
    for name in ("crater", "crater_big0", "crater_big1", "crater_big2", "crater_big3", "bay0", "bay1", "bayo0", "bayo1",
                 "crater_bay0", "crater_bay1"):
        defines[f"T_{name.upper()}"] = hids[name]
    assert len(ht) <= 256
    return {
        "n": 3,
        "palettes": pal,
        "items": [
            {"name": "tiles", "frames": gt, "pal": ocean.OCEAN},
            {"name": "map", "data": gmap.astype(np.uint8).tobytes()},
            {"name": "sky_tiles", "frames": st, "pal": ocean.SKY},
            {"name": "sky_map", "data": smap.astype(np.uint8).tobytes()},
            {"name": "hull_tiles", "frames": ht, "pal": ship.HULL},
            {"name": "hull_map", "data": hmap.astype(np.uint8).tobytes()},
            {"name": "boss_bridge", "frames": ship.bridge(), "pal": ship.BOSS3},
            {"name": "boss_reactor", "frames": ship.reactor(), "pal": ship.BOSS3},
            {"name": "boss_gun", "frames": ship.main_gun(), "pal": ship.BOSS3},
            {"name": "boss_turret", "frames": boss.turret(), "pal": ship.BOSS3},
        ],
        "defines": defines,
        "data_h": ground_table(3, units),
        "previews": {"ground": (tilemap_image(gt, gmap), ocean.OCEAN), "sky": (tilemap_image(st, smap), ocean.SKY),
                     "hull": (tilemap_image(ht, hmap), ship.HULL)},
    }


def all_stages():
    return [stage1(), stage2(), stage3()]
