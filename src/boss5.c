#include "boss.h"
#include "bosses.h"
#include "bullets.h"
#include "enemies.h"
#include "game.h"
#include "xram.h"
#include "gen/stage5.h"

/*
 * Stage 5: GLACIER CRAWLER. A tracked fortress grinding up the ice with
 * the player in front of it. Its hull is plane 1's tile layer, and its
 * treads run by palette cycling. Twin guns, four turrets, launch hatches
 * that loose fighters, and an ice drill at the front that sprays pink;
 * the cryo reactor at the rear is the core. Scripted: motion and patterns.
 */

enum
{
    BP_HATCH_L,
    BP_HATCH_R,
    BP_TUR0,
    BP_TUR1,
    BP_TUR2,
    BP_TUR3,
    BP_GUN_L,
    BP_GUN_R,
    BP_DRILL,
    BP_REACTOR,
    BP_N
};

#define IMG(name) (XRAM_BOSS + B5_##name)
#define AT(name) S5_##name##_OX, S5_##name##_OY
#define TUR(n) {AT(TUR##n), 16, 16, 7, 7, 60, 4, IMG(BOSS_TURRET), 0, PK_TURRET, NO_PARENT, 1000}
#define GUN(n) {AT(n), 32, 32, 12, 12, 180, 12, IMG(BOSS_GUN), 0, PK_TURRET, NO_PARENT, 3000}

static const part_t parts[BP_N] = {
    [BP_HATCH_L] = {AT(HATCH_L), 16, 32, 7, 14, 110, 6, 0, 0, PK_HIDDEN, NO_PARENT, 2000},
    [BP_HATCH_R] = {AT(HATCH_R), 16, 32, 7, 14, 110, 6, 0, 0, PK_HIDDEN, NO_PARENT, 2000},
    [BP_TUR0] = TUR(0),
    [BP_TUR1] = TUR(1),
    [BP_TUR2] = TUR(2),
    [BP_TUR3] = TUR(3),
    [BP_GUN_L] = GUN(GUN_L),
    [BP_GUN_R] = GUN(GUN_R),
    [BP_DRILL] = {AT(DRILL), 32, 32, 11, 13, 260, 16, IMG(BOSS_DRILL), 0, PK_SPRITE, NO_PARENT, 4000},
    [BP_REACTOR] = {AT(REACTOR), 32, 32, 12, 12, 560, 24, IMG(BOSS_REACTOR), 0, PK_SPRITE, NO_PARENT, 10000},
};

#define SOCK(name) {S5_##name##_COL, S5_##name##_ROW}
static const uint8_t socket[BP_N][2] = {
    SOCK(HATCH_L), SOCK(HATCH_R), SOCK(TUR0), SOCK(TUR1), SOCK(TUR2),
    SOCK(TUR3), SOCK(GUN_L), SOCK(GUN_R), SOCK(DRILL), SOCK(REACTOR),
};

static const hull_t hull = {
    .map = XRAM_BOSS + B5_HULL_MAP,
    .tiles = XRAM_BOSS + B5_HULL_TILES,
    .w = S5_HULL_W,
    .h = S5_HULL_H,
    .socket = socket,
    .crater = S5_T_CRATER,
    .crater_big = S5_T_CRATER_BIG0,
};

static const motion_t motion = {
    .enter_speed = 12,
    .y = 80,
    .amp_x = 14,
    .amp_y = 10,
    .rate_x = 2,
    .rate_y = 3,
};

static const pattern_t patterns[] = {
    /* part, kind, bullet, speed, period, phase, count, step, when, arg */
    {BP_TUR0, PT_AIM, BK_BLUE, 28, 90, 0, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR1, PT_AIM, BK_BLUE, 28, 90, 22, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR2, PT_AIM, BK_BLUE, 28, 90, 45, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR3, PT_AIM, BK_BLUE, 28, 90, 67, 1, 0, WHEN_ALWAYS, 0},
    {BP_GUN_L, PT_AIM, BK_BLUE_BIG, 24, 140, 0, 5, 12, WHEN_ALWAYS, 0},
    {BP_GUN_R, PT_AIM, BK_BLUE_BIG, 24, 140, 70, 5, 12, WHEN_ALWAYS, 0},
    {BP_DRILL, PT_DOWN, BK_PINK, 26, 100, 50, 7, 10, WHEN_ALWAYS, 0},
    {BP_HATCH_L, PT_SPAWN, 0, 0, 200, 40, 1, COL_RED, WHEN_ALWAYS, ET_DART},
    {BP_HATCH_R, PT_SPAWN, 0, 0, 200, 140, 1, COL_BLUE, WHEN_ALWAYS, ET_DART},
    /* The reactor wakes once the drill is gone, or late in the fight. */
    {BP_REACTOR, PT_RING, BK_PINK, 20, 80, 20, 12, 7, WHEN_DEAD, BP_DRILL},
    {BP_REACTOR, PT_RING, BK_BLUE, 22, 70, 55, 14, 5, WHEN_LATE, 0},
    {BP_REACTOR, PT_SPIRAL, BK_PINK, 22, 7, 0, 3, 9, WHEN_ANGRY, 0},
};

static void frame(uint8_t i, mode5_csprite_t *s)
{
    if (part_dead[i])
        return;
    if (i == BP_DRILL)
        s->xram_sprite_ptr = IMG(BOSS_DRILL) + ((boss_t >> 2) & 1) * B5_BOSS_DRILL_FRAME_BYTES;
    else if (i == BP_REACTOR)
        s->xram_sprite_ptr = IMG(BOSS_REACTOR) + ((boss_t >> 3) & 1) * B5_BOSS_REACTOR_FRAME_BYTES;
}

const boss_def_t boss5 = {
    .portrait = IMG(PORTRAIT),
    .core_label = "REACTOR",
    .parts = parts,
    .n = BP_N,
    .core = BP_REACTOR,
    .time_limit = 6000,
    .frame = frame,
    .motion = &motion,
    .patterns = patterns,
    .n_patterns = sizeof(patterns) / sizeof(patterns[0]),
    .hull = &hull,
};
