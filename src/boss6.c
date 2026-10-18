#include "boss.h"
#include "bosses.h"
#include "bullets.h"
#include "game.h"
#include "xram.h"
#include "gen/stage6.h"

/*
 * Stage 6: HALO STATION. An orbital ring station, its hull plane 1's tile
 * layer, running lights chasing round the ring. Six turrets on the ring,
 * four gun satellites orbiting outside it, faster as they fall, and the
 * core in the hub. Scripted: motion, orbit and patterns.
 */

enum
{
    BP_TUR0,
    BP_TUR1,
    BP_TUR2,
    BP_TUR3,
    BP_TUR4,
    BP_TUR5,
    BP_SAT0,
    BP_SAT1,
    BP_SAT2,
    BP_SAT3,
    BP_CORE,
    BP_N
};

#define IMG(name) (XRAM_BOSS + B6_##name)
#define AT(name) S6_##name##_OX, S6_##name##_OY
#define TUR(n) {AT(TUR##n), 16, 16, 7, 7, 70, 5, IMG(BOSS_TURRET), 0, PK_TURRET, NO_PARENT, 1000}
#define SAT {0, 0, 32, 32, 10, 10, 110, 10, IMG(BOSS_SAT), 0, PK_SPRITE, NO_PARENT, 2500}

static const part_t parts[BP_N] = {
    TUR(0), TUR(1), TUR(2), TUR(3), TUR(4), TUR(5),
    SAT, SAT, SAT, SAT,
    [BP_CORE] = {AT(CORE), 32, 32, 12, 12, 620, 30, IMG(BOSS_CORE), 0, PK_SPRITE, NO_PARENT, 10000},
};

#define SOCK(name) {S6_##name##_COL, S6_##name##_ROW}
static const uint8_t socket[BP_N][2] = {
    SOCK(TUR0), SOCK(TUR1), SOCK(TUR2), SOCK(TUR3), SOCK(TUR4), SOCK(TUR5),
    {NO_SOCKET}, {NO_SOCKET}, {NO_SOCKET}, {NO_SOCKET}, /* the satellites are not on the hull */
    SOCK(CORE),
};

static const hull_t hull = {
    .map = XRAM_BOSS + B6_HULL_MAP,
    .tiles = XRAM_BOSS + B6_HULL_TILES,
    .w = S6_HULL_W,
    .h = S6_HULL_H,
    .socket = socket,
    .crater = S6_T_CRATER,
    .crater_big = S6_T_CRATER_BIG0,
};

static const motion_t motion = {
    .enter_speed = 12,
    .y = 76,
    .amp_x = 16,
    .amp_y = 8,
    .rate_x = 2,
    .rate_y = 2,
    .orbit_first = BP_SAT0,
    .orbit_n = 4,
    .orbit_rx = 86,
    .orbit_ry = 74,
};

static const pattern_t patterns[] = {
    /* part, kind, bullet, speed, period, phase, count, step, when, arg */
    {BP_TUR0, PT_AIM, BK_BLUE, 28, 96, 0, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR1, PT_AIM, BK_BLUE, 28, 96, 16, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR2, PT_AIM, BK_BLUE, 28, 96, 32, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR3, PT_AIM, BK_BLUE, 28, 96, 48, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR4, PT_AIM, BK_BLUE, 28, 96, 64, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR5, PT_AIM, BK_BLUE, 28, 96, 80, 1, 0, WHEN_ALWAYS, 0},
    {BP_SAT0, PT_AIM, BK_PINK, 26, 120, 0, 3, 14, WHEN_ALWAYS, 0},
    {BP_SAT1, PT_AIM, BK_PINK, 26, 120, 30, 3, 14, WHEN_ALWAYS, 0},
    {BP_SAT2, PT_AIM, BK_PINK, 26, 120, 60, 3, 14, WHEN_ALWAYS, 0},
    {BP_SAT3, PT_AIM, BK_PINK, 26, 120, 90, 3, 14, WHEN_ALWAYS, 0},
    {BP_CORE, PT_RING, BK_BLUE_BIG, 18, 150, 75, 10, 13, WHEN_ALWAYS, 0},
    {BP_CORE, PT_SPIRAL, BK_PINK, 22, 6, 0, 4, 7, WHEN_ANGRY, 0},
    {BP_CORE, PT_RING, BK_PINK, 22, 60, 30, 16, 6, WHEN_LATE, 0},
};

static void frame(uint8_t i, mode5_csprite_t *s)
{
    if (part_dead[i])
        return;
    if (i >= BP_SAT0 && i <= BP_SAT3)
        s->xram_sprite_ptr = IMG(BOSS_SAT) + (((boss_t >> 3) + i) & 1) * B6_BOSS_SAT_FRAME_BYTES;
    else if (i == BP_CORE)
        s->xram_sprite_ptr = IMG(BOSS_CORE) + ((boss_t >> 3) & 1) * B6_BOSS_CORE_FRAME_BYTES;
}

const boss_def_t boss6 = {
    .portrait = IMG(PORTRAIT),
    .core_label = "CORE",
    .parts = parts,
    .n = BP_N,
    .core = BP_CORE,
    .time_limit = 6000,
    .frame = frame,
    .motion = &motion,
    .patterns = patterns,
    .n_patterns = sizeof(patterns) / sizeof(patterns[0]),
    .hull = &hull,
};
