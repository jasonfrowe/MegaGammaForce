#include "boss.h"
#include "bosses.h"
#include "bullets.h"
#include "enemies.h"
#include "game.h"
#include "xram.h"
#include "gen/stage7.h"

/*
 * Stage 7: MASS DRIVER. A hovering gun platform built round a rail cannon
 * that runs its whole length, plane 1's tile layer, its coils charging by
 * palette cycling. Every few seconds the muzzle glows and fires a burst of
 * fast slugs straight down: move out from under it. Capacitor pods spray
 * pink, turrets and launch hatches cover the rest. The breech at the rear
 * is the core. Scripted: motion and patterns.
 */

enum
{
    BP_HATCH_L,
    BP_HATCH_R,
    BP_TUR0,
    BP_TUR1,
    BP_TUR2,
    BP_TUR3,
    BP_CAP_L,
    BP_CAP_R,
    BP_MUZZLE,
    BP_BREECH,
    BP_N
};

#define IMG(name) (XRAM_BOSS + B7_##name)
#define AT(name) S7_##name##_OX, S7_##name##_OY
#define TUR(n) {AT(TUR##n), 16, 16, 7, 7, 70, 4, IMG(BOSS_TURRET), 0, PK_TURRET, NO_PARENT, 1000}
#define CAP(n) {AT(n), 32, 32, 12, 12, 170, 12, IMG(BOSS_CAP), 0, PK_SPRITE, NO_PARENT, 3000}
#define HATCH(n) {AT(n), 16, 32, 7, 14, 110, 5, 0, 0, PK_HIDDEN, NO_PARENT, 2000}

static const part_t parts[BP_N] = {
    HATCH(HATCH_L), HATCH(HATCH_R),
    TUR(0), TUR(1), TUR(2), TUR(3),
    CAP(CAP_L), CAP(CAP_R),
    [BP_MUZZLE] = {AT(MUZZLE), 32, 32, 11, 13, 260, 18, IMG(BOSS_MUZZLE), 0, PK_SPRITE, NO_PARENT, 4000},
    [BP_BREECH] = {AT(BREECH), 32, 32, 12, 12, 640, 24, IMG(BOSS_BREECH), 0, PK_SPRITE, NO_PARENT, 10000},
};

#define SOCK(name) {S7_##name##_COL, S7_##name##_ROW}
static const uint8_t socket[BP_N][2] = {
    SOCK(HATCH_L), SOCK(HATCH_R), SOCK(TUR0), SOCK(TUR1), SOCK(TUR2),
    SOCK(TUR3), SOCK(CAP_L), SOCK(CAP_R), SOCK(MUZZLE), SOCK(BREECH),
};

static const hull_t hull = {
    .map = XRAM_BOSS + B7_HULL_MAP,
    .tiles = XRAM_BOSS + B7_HULL_TILES,
    .w = S7_HULL_W,
    .h = S7_HULL_H,
    .socket = socket,
    .crater = S7_T_CRATER,
    .crater_big = S7_T_CRATER_BIG0,
};

/* Sways wide and slowly, so the slugs sweep across the screen. */
static const motion_t motion = {
    .enter_speed = 12,
    .y = 78,
    .amp_x = 18,
    .amp_y = 8,
    .rate_x = 2,
    .rate_y = 3,
};

#define SLUG_PERIOD 160
#define SLUG(ph) {BP_MUZZLE, PT_DOWN, BK_NEEDLE, 60, SLUG_PERIOD, ph, 3, 6, WHEN_ALWAYS, 0}

static const pattern_t patterns[] = {
    /* part, kind, bullet, speed, period, phase, count, step, when, arg */
    SLUG(0), SLUG(4), SLUG(8), SLUG(12), SLUG(16), SLUG(20),
    {BP_TUR0, PT_AIM, BK_BLUE, 28, 90, 30, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR1, PT_AIM, BK_BLUE, 28, 90, 52, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR2, PT_AIM, BK_BLUE, 28, 90, 75, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR3, PT_AIM, BK_BLUE, 28, 90, 7, 1, 0, WHEN_ALWAYS, 0},
    {BP_CAP_L, PT_AIM, BK_PINK, 24, 120, 60, 5, 12, WHEN_ALWAYS, 0},
    {BP_CAP_R, PT_AIM, BK_PINK, 24, 120, 100, 5, 12, WHEN_ALWAYS, 0},
    {BP_HATCH_L, PT_SPAWN, 0, 0, 220, 70, 1, COL_YELLOW, WHEN_ALWAYS, ET_DART},
    {BP_HATCH_R, PT_SPAWN, 0, 0, 220, 180, 1, COL_RED, WHEN_ALWAYS, ET_DART},
    /* With the muzzle gone, the charge has nowhere to go but out. */
    {BP_BREECH, PT_SPIRAL, BK_PINK, 22, 8, 0, 2, 11, WHEN_DEAD, BP_MUZZLE},
    {BP_BREECH, PT_RING, BK_BLUE_BIG, 18, 100, 50, 12, 9, WHEN_ANGRY, 0},
    {BP_BREECH, PT_RING, BK_PINK, 22, 70, 20, 16, 6, WHEN_LATE, 0},
};

static void frame(uint8_t i, mode5_csprite_t *s)
{
    if (part_dead[i])
        return;
    /* The muzzle glows as the next burst charges. */
    if (i == BP_MUZZLE)
        s->xram_sprite_ptr = IMG(BOSS_MUZZLE) + ((boss_t % SLUG_PERIOD) >= SLUG_PERIOD - 40 && (boss_t & 4)) *
                                                    B7_BOSS_MUZZLE_FRAME_BYTES;
    else if (i == BP_CAP_L || i == BP_CAP_R)
        s->xram_sprite_ptr = IMG(BOSS_CAP) + (((boss_t >> 3) + i) & 1) * B7_BOSS_CAP_FRAME_BYTES;
    else if (i == BP_BREECH)
        s->xram_sprite_ptr = IMG(BOSS_BREECH) + ((boss_t >> 3) & 1) * B7_BOSS_BREECH_FRAME_BYTES;
}

const boss_def_t boss7 = {
    .portrait = IMG(PORTRAIT),
    .core_label = "BREECH",
    .parts = parts,
    .n = BP_N,
    .core = BP_BREECH,
    .time_limit = 6000,
    .frame = frame,
    .motion = &motion,
    .patterns = patterns,
    .n_patterns = sizeof(patterns) / sizeof(patterns[0]),
    .hull = &hull,
};
