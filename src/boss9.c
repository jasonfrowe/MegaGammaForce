#include "boss.h"
#include "bosses.h"
#include "bullets.h"
#include "game.h"
#include "xram.h"
#include "gen/stage9.h"

/*
 * Stage 9: GAMMA PRIME, the last boss. The Gamma Core's true form, a
 * sunburst fortress feeding on the Sun: gold plating on plane 1's tile
 * layer with light running down its spine, four prisms orbiting it, ring
 * cannons either side, a beam emitter below and the core at its heart.
 * Scripted: motion, orbit and patterns, more of them as it breaks up.
 */

enum
{
    BP_TUR0,
    BP_TUR1,
    BP_TUR2,
    BP_TUR3,
    BP_PRISM0,
    BP_PRISM1,
    BP_PRISM2,
    BP_PRISM3,
    BP_CANNON_L,
    BP_CANNON_R,
    BP_EMITTER,
    BP_CORE,
    BP_N
};

#define IMG(name) (XRAM_BOSS + B9_##name)
#define AT(name) S9_##name##_OX, S9_##name##_OY
#define TUR(n) {AT(TUR##n), 16, 16, 7, 7, 80, 4, IMG(BOSS_TURRET), 0, PK_TURRET, NO_PARENT, 1000}
#define PRISM {0, 0, 32, 32, 10, 10, 120, 6, IMG(BOSS_PRISM), 0, PK_SPRITE, NO_PARENT, 2500}
#define CANNON(n) {AT(n), 32, 32, 12, 12, 220, 12, IMG(BOSS_CANNON), 0, PK_SPRITE, NO_PARENT, 4000}

static const part_t parts[BP_N] = {
    TUR(0), TUR(1), TUR(2), TUR(3),
    PRISM, PRISM, PRISM, PRISM,
    CANNON(CANNON_L), CANNON(CANNON_R),
    [BP_EMITTER] = {AT(EMITTER), 32, 32, 11, 13, 300, 14, IMG(BOSS_EMITTER), 0, PK_SPRITE, NO_PARENT, 5000},
    [BP_CORE] = {AT(CORE), 32, 32, 12, 12, 1000, 26, IMG(BOSS_CORE), 0, PK_SPRITE, NO_PARENT, 20000},
};

#define SOCK(name) {S9_##name##_COL, S9_##name##_ROW}
static const uint8_t socket[BP_N][2] = {
    SOCK(TUR0), SOCK(TUR1), SOCK(TUR2), SOCK(TUR3),
    {NO_SOCKET}, {NO_SOCKET}, {NO_SOCKET}, {NO_SOCKET}, /* the prisms orbit outside */
    SOCK(CANNON_L), SOCK(CANNON_R), SOCK(EMITTER), SOCK(CORE),
};

static const hull_t hull = {
    .map = XRAM_BOSS + B9_HULL_MAP,
    .tiles = XRAM_BOSS + B9_HULL_TILES,
    .w = S9_HULL_W,
    .h = S9_HULL_H,
    .socket = socket,
    .crater = S9_T_CRATER,
    .crater_big = S9_T_CRATER_BIG0,
};

static const motion_t motion = {
    .enter_speed = 10,
    .y = 78,
    .amp_x = 16,
    .amp_y = 10,
    .rate_x = 2,
    .rate_y = 2,
    .orbit_first = BP_PRISM0,
    .orbit_n = 4,
    .orbit_rx = 92,
    .orbit_ry = 78,
};

#define BEAM_PERIOD 150

static const pattern_t patterns[] = {
    /* part, kind, bullet, speed, period, phase, count, step, when, arg */
    {BP_TUR0, PT_AIM, BK_BLUE, 28, 84, 0, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR1, PT_AIM, BK_BLUE, 28, 84, 21, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR2, PT_AIM, BK_BLUE, 28, 84, 42, 1, 0, WHEN_ALWAYS, 0},
    {BP_TUR3, PT_AIM, BK_BLUE, 28, 84, 63, 1, 0, WHEN_ALWAYS, 0},
    {BP_PRISM0, PT_AIM, BK_PINK, 26, 160, 0, 3, 12, WHEN_ALWAYS, 0},
    {BP_PRISM1, PT_AIM, BK_PINK, 26, 160, 40, 3, 12, WHEN_ALWAYS, 0},
    {BP_PRISM2, PT_AIM, BK_PINK, 26, 160, 80, 3, 12, WHEN_ALWAYS, 0},
    {BP_PRISM3, PT_AIM, BK_PINK, 26, 160, 120, 3, 12, WHEN_ALWAYS, 0},
    {BP_CANNON_L, PT_RING, BK_PINK, 20, 130, 30, 10, 7, WHEN_ALWAYS, 0},
    {BP_CANNON_R, PT_RING, BK_PINK, 20, 130, 95, 10, 7, WHEN_ALWAYS, 0},
    /* The emitter glows, then fires three quick fans of needles straight down. */
    {BP_EMITTER, PT_DOWN, BK_NEEDLE, 52, BEAM_PERIOD, 0, 7, 8, WHEN_ALWAYS, 0},
    {BP_EMITTER, PT_DOWN, BK_NEEDLE, 52, BEAM_PERIOD, 6, 6, 8, WHEN_ALWAYS, 0},
    {BP_EMITTER, PT_DOWN, BK_NEEDLE, 52, BEAM_PERIOD, 12, 7, 8, WHEN_ALWAYS, 0},
    {BP_CORE, PT_RING, BK_BLUE_BIG, 16, 170, 85, 12, 10, WHEN_ALWAYS, 0},
    /* Its emitter gone, the core pours its light out in a spiral... */
    {BP_CORE, PT_SPIRAL, BK_PINK, 20, 9, 0, 2, 13, WHEN_DEAD, BP_EMITTER},
    /* ...and hurt, it burns. */
    {BP_CORE, PT_SPIRAL, BK_BLUE, 22, 6, 3, 3, 7, WHEN_ANGRY, 0},
    {BP_CORE, PT_RING, BK_PINK, 22, 70, 35, 18, 5, WHEN_LATE, 0},
};

static void frame(uint8_t i, mode5_csprite_t *s)
{
    if (part_dead[i])
        return;
    if (i >= BP_PRISM0 && i <= BP_PRISM3)
        s->xram_sprite_ptr = IMG(BOSS_PRISM) + (((boss_t >> 3) + i) & 1) * B9_BOSS_PRISM_FRAME_BYTES;
    else if (i == BP_CANNON_L || i == BP_CANNON_R)
        s->xram_sprite_ptr = IMG(BOSS_CANNON) + (((boss_t >> 4) + i) & 1) * B9_BOSS_CANNON_FRAME_BYTES;
    else if (i == BP_EMITTER) /* glowing as the beam charges */
        s->xram_sprite_ptr = IMG(BOSS_EMITTER) +
                             ((boss_t % BEAM_PERIOD) >= BEAM_PERIOD - 40 && (boss_t & 4)) * B9_BOSS_EMITTER_FRAME_BYTES;
    else if (i == BP_CORE)
        s->xram_sprite_ptr = IMG(BOSS_CORE) + ((boss_t >> 3) & 1) * B9_BOSS_CORE_FRAME_BYTES;
}

const boss_def_t boss9 = {
    .portrait = IMG(PORTRAIT),
    .core_label = "GAMMA",
    .parts = parts,
    .n = BP_N,
    .core = BP_CORE,
    .time_limit = 7200,
    .frame = frame,
    .motion = &motion,
    .patterns = patterns,
    .n_patterns = sizeof(patterns) / sizeof(patterns[0]),
    .hull = &hull,
};
