#include "boss.h"
#include "bosses.h"
#include "bullets.h"
#include "enemies.h"
#include "game.h"
#include "xram.h"
#include "gen/stage8.h"

/*
 * Stage 8: HIVE MOTHERSHIP. An alien ship grown, not built: a carapace of
 * honeycomb on plane 1's tile layer, veins pulsing out from the great eye
 * at its heart, which is the core. Two spore pods launch saucers, two
 * mandibles spray pink, four turrets cover the rest. The eye glares before
 * each blue ring. Scripted: motion and patterns.
 */

enum
{
    BP_TUR0,
    BP_TUR1,
    BP_TUR2,
    BP_TUR3,
    BP_POD_L,
    BP_POD_R,
    BP_CLAW_L,
    BP_CLAW_R,
    BP_EYE,
    BP_N
};

#define IMG(name) (XRAM_BOSS + B8_##name)
#define AT(name) S8_##name##_OX, S8_##name##_OY
#define TUR(n) {AT(TUR##n), 16, 16, 7, 7, 70, 4, IMG(BOSS_TURRET), 0, PK_TURRET, NO_PARENT, 1000}
#define POD(n) {AT(n), 32, 32, 12, 12, 180, 12, IMG(BOSS_POD), 0, PK_SPRITE, NO_PARENT, 3000}
#define CLAW(n) {AT(n), 32, 32, 11, 13, 200, 14, IMG(BOSS_CLAW), 0, PK_SPRITE, NO_PARENT, 3000}

static const part_t parts[BP_N] = {
    TUR(0), TUR(1), TUR(2), TUR(3),
    POD(POD_L), POD(POD_R),
    CLAW(CLAW_L), CLAW(CLAW_R),
    [BP_EYE] = {AT(EYE), 32, 32, 12, 10, 680, 24, IMG(BOSS_EYE), 0, PK_SPRITE, NO_PARENT, 10000},
};

#define SOCK(name) {S8_##name##_COL, S8_##name##_ROW}
static const uint8_t socket[BP_N][2] = {
    SOCK(TUR0), SOCK(TUR1), SOCK(TUR2), SOCK(TUR3), SOCK(POD_L),
    SOCK(POD_R), SOCK(CLAW_L), SOCK(CLAW_R), SOCK(EYE),
};

static const hull_t hull = {
    .map = XRAM_BOSS + B8_HULL_MAP,
    .tiles = XRAM_BOSS + B8_HULL_TILES,
    .w = S8_HULL_W,
    .h = S8_HULL_H,
    .socket = socket,
    .crater = S8_T_CRATER,
    .crater_big = S8_T_CRATER_BIG0,
};

/* Drifts in a slow loop, as if swimming. */
static const motion_t motion = {
    .enter_speed = 10,
    .y = 76,
    .amp_x = 20,
    .amp_y = 12,
    .rate_x = 2,
    .rate_y = 1,
};

#define RING_PERIOD 140

static const pattern_t patterns[] = {
    /* part, kind, bullet, speed, period, phase, count, step, when, arg */
    {BP_TUR0, PT_AIM, BK_BLUE, 28, 90, 0, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR1, PT_AIM, BK_BLUE, 28, 90, 45, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR2, PT_AIM, BK_BLUE, 28, 90, 22, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR3, PT_AIM, BK_BLUE, 28, 90, 67, 1, 0, WHEN_ALWAYS, 0},
    {BP_POD_L, PT_SPAWN, 0, 0, 200, 40, 1, COL_RED, WHEN_ALWAYS, ET_SAUCER},
    {BP_POD_R, PT_SPAWN, 0, 0, 200, 140, 1, COL_BLUE, WHEN_ALWAYS, ET_SAUCER},
    {BP_CLAW_L, PT_AIM, BK_PINK, 24, 110, 30, 5, 12, WHEN_ALWAYS, 0},
    {BP_CLAW_R, PT_AIM, BK_PINK, 24, 110, 85, 5, 12, WHEN_ALWAYS, 0},
    {BP_EYE, PT_RING, BK_BLUE_BIG, 18, RING_PERIOD, 0, 12, 10, WHEN_ALWAYS, 0},
    /* A mandible gone, it spits needles from the maw. */
    {BP_EYE, PT_DOWN, BK_NEEDLE, 44, 50, 25, 5, 10, WHEN_DEAD, BP_CLAW_L},
    {BP_EYE, PT_SPIRAL, BK_PINK, 22, 7, 0, 3, 9, WHEN_ANGRY, 0},
    {BP_EYE, PT_RING, BK_PINK, 22, 64, 32, 16, 6, WHEN_LATE, 0},
};

static void frame(uint8_t i, mode5_csprite_t *s)
{
    if (part_dead[i])
        return;
    /* The eye glares as the next ring comes. */
    if (i == BP_EYE)
        s->xram_sprite_ptr = IMG(BOSS_EYE) + ((boss_t % RING_PERIOD) >= RING_PERIOD - 30) * B8_BOSS_EYE_FRAME_BYTES;
    else if (i == BP_POD_L || i == BP_POD_R)
        s->xram_sprite_ptr = IMG(BOSS_POD) + (((boss_t >> 4) + i) & 1) * B8_BOSS_POD_FRAME_BYTES;
    else if (i == BP_CLAW_L || i == BP_CLAW_R)
        s->xram_sprite_ptr = IMG(BOSS_CLAW) + (((boss_t >> 3) + i) & 1) * B8_BOSS_CLAW_FRAME_BYTES;
}

const boss_def_t boss8 = {
    .portrait = IMG(PORTRAIT),
    .core_label = "EYE",
    .parts = parts,
    .n = BP_N,
    .core = BP_EYE,
    .time_limit = 6000,
    .frame = frame,
    .motion = &motion,
    .patterns = patterns,
    .n_patterns = sizeof(patterns) / sizeof(patterns[0]),
    .hull = &hull,
};
