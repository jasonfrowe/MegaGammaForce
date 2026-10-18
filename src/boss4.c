#include "boss.h"
#include "bosses.h"
#include "bullets.h"
#include "game.h"
#include "xram.h"
#include "gen/stage4.h"

/*
 * Stage 4: DUNE WYRM. A sandworm whose head winds a figure of eight while
 * nine armoured segments and a tail follow its path. The head is the core;
 * every segment can be shot off. Scripted: motion and patterns only.
 */

enum
{
    BP_TAIL, /* the end of the chain */
    BP_SEG0, /* ...nine segments, BP_SEG0 + 8 nearest the head */
    BP_HEAD = BP_SEG0 + 9,
    BP_N
};

#define IMG(name) (XRAM_BOSS + B4_##name)
#define SEG {0, 0, 32, 32, 11, 11, 70, 7, IMG(BOSS_SEG), 0, PK_SPRITE, NO_PARENT, 1500}

static const part_t parts[BP_N] = {
    [BP_TAIL] = {0, 0, 32, 32, 10, 12, 120, 12, IMG(BOSS_TAIL), 0, PK_SPRITE, NO_PARENT, 3000},
    SEG, SEG, SEG, SEG, SEG, SEG, SEG, SEG, SEG,
    [BP_HEAD] = {0, 0, 48, 48, 17, 17, 600, 25, IMG(BOSS_HEAD), 0, PK_SPRITE, NO_PARENT, 10000},
};

static const motion_t motion = {
    .enter_speed = 16,
    .y = 84,
    .amp_x = 62,
    .amp_y = 44,
    .rate_x = 0,
    .rate_y = 1,
    .chain_first = BP_TAIL,
    .chain_n = 10,
    .chain_gap = 10,
};

#define B(n) (BP_SEG0 + (n))
static const pattern_t patterns[] = {
    /* part, kind, bullet, speed, period, phase, count, step, when, arg */
    {BP_HEAD, PT_DOWN, BK_PINK, 26, 80, 0, 5, 12, WHEN_ALWAYS, 0},
    {BP_HEAD, PT_AIM, BK_BLUE_BIG, 22, 120, 60, 3, 16, WHEN_ALWAYS, 0},
    {B(1), PT_AIM, BK_BLUE, 28, 100, 0, 1, 0, WHEN_ALWAYS, 0},
    {B(3), PT_AIM, BK_BLUE, 28, 100, 25, 1, 0, WHEN_ALWAYS, 0},
    {B(5), PT_AIM, BK_BLUE, 28, 100, 50, 1, 0, WHEN_ALWAYS, 0},
    {B(7), PT_AIM, BK_BLUE, 28, 100, 75, 1, 0, WHEN_ALWAYS, 0},
    {BP_TAIL, PT_RING, BK_PINK, 20, 90, 45, 10, 9, WHEN_ALWAYS, 0},
    {BP_HEAD, PT_SPIRAL, BK_PINK, 24, 6, 0, 3, 11, WHEN_ANGRY, 0},
    {BP_HEAD, PT_RING, BK_BLUE, 22, 70, 35, 16, 5, WHEN_LATE, 0},
};

static void frame(uint8_t i, mode5_csprite_t *s)
{
    if (i >= BP_SEG0 && i < BP_HEAD)
        s->xram_sprite_ptr = IMG(BOSS_SEG) + (i & 1) * B4_BOSS_SEG_FRAME_BYTES;
}

const boss_def_t boss4 = {
    .portrait = IMG(PORTRAIT),
    .core_label = "HEAD",
    .parts = parts,
    .n = BP_N,
    .core = BP_HEAD,
    .time_limit = 5400,
    .frame = frame,
    .motion = &motion,
    .patterns = patterns,
    .n_patterns = sizeof(patterns) / sizeof(patterns[0]),
};
