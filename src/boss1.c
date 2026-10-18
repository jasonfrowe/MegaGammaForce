#include "boss.h"
#include "bosses.h"
#include "bullets.h"
#include "game.h"
#include "player.h"
#include "sfx.h"
#include "util.h"
#include "xram.h"
#include "gen/stage1.h"

/*
 * Stage 1: GAMMA CORE. A fortress-ship of eight parts: the core, two
 * wings carrying two turrets each, and a belly cannon.
 */

enum
{
    BP_CANNON,
    BP_WING_L,
    BP_WING_R,
    BP_CORE,
    BP_TUR0,
    BP_TUR1,
    BP_TUR2,
    BP_TUR3,
    BP_N
};

#define IMG(name) (XRAM_BANK + S1_##name)

static const part_t parts[BP_N] = {
    [BP_CANNON] = {0, 44, 32, 32, 12, 10, 150, 12, IMG(BOSS_CANNON), 0, PK_SPRITE, NO_PARENT, 2000},
    [BP_WING_L] = {-48, -4, 32, 64, 13, 28, 180, 15, IMG(BOSS_WING), 0, PK_SPRITE, NO_PARENT, 2000},
    [BP_WING_R] = {48, -4, 32, 64, 13, 28, 180, 15, IMG(BOSS_WING), MODE5_HFLIP, PK_SPRITE, NO_PARENT, 2000},
    [BP_CORE] = {0, 0, 64, 64, 17, 17, 450, 30, IMG(BOSS_CORE), 0, PK_SPRITE, NO_PARENT, 10000},
    [BP_TUR0] = {-48, -16, 16, 16, 7, 7, 60, 7, IMG(BOSS_TURRET), 0, PK_TURRET, BP_WING_L, 2000},
    [BP_TUR1] = {-48, 6, 16, 16, 7, 7, 60, 7, IMG(BOSS_TURRET), 0, PK_TURRET, BP_WING_L, 2000},
    [BP_TUR2] = {47, -16, 16, 16, 7, 7, 60, 7, IMG(BOSS_TURRET), 0, PK_TURRET, BP_WING_R, 2000},
    [BP_TUR3] = {47, 6, 16, 16, 7, 7, 60, 7, IMG(BOSS_TURRET), 0, PK_TURRET, BP_WING_R, 2000},
};

static uint8_t spin;

static void start(void)
{
    spin = 0;
}

static bool enter(void)
{
    boss_y += 16;
    return boss_y >= Q4(66);
}

static void attack(void)
{
    if (pl_state != PL_ALIVE)
        return;
    uint16_t t = boss_t;
    int16_t px = pl_x >> 4, py = pl_y >> 4;

    /* Turrets: aimed blue shots, staggered. */
    for (uint8_t i = BP_TUR0; i <= BP_TUR3; i++) {
        if (part_dead[i])
            continue;
        uint8_t a = boss_aim(i);
        if ((uint8_t)(t + i * 17) % 64 == 0)
            bullet_fire(part_px[i], part_py[i], a, 30, BK_BLUE);
    }
    /* Wings: pink fans, alternating sides. */
    if (!part_dead[BP_WING_L] && t % 100 == 0)
        bullet_fan(part_px[BP_WING_L], part_py[BP_WING_L] + 28, ANG_DOWN + 8, 28, BK_PINK, 5, 12);
    if (!part_dead[BP_WING_R] && t % 100 == 50)
        bullet_fan(part_px[BP_WING_R], part_py[BP_WING_R] + 28, ANG_DOWN - 8, 28, BK_PINK, 5, 12);
    /* Cannon: charges, then lobs big pink orbs. Absorb them with the sword! */
    if (!part_dead[BP_CANNON] && t % 180 == 0) {
        int16_t x = part_px[BP_CANNON], y = part_py[BP_CANNON] + 14;
        bullet_fan(x, y, angle_to(px - x, py - y), 22, BK_PINK_BIG, 3, 18);
        sfx_enemy(SFX_ENEMY_FIRE, SFX_PRI_EVENT);
    }
    /* Core: spirals once it is exposed, or once time drags on. */
    uint8_t wings = (uint8_t)(part_dead[BP_WING_L] + part_dead[BP_WING_R]);
    bool angry = part_hp[BP_CORE] < parts[BP_CORE].hp / 2;
    if (wings || t > 1500 || angry) {
        uint8_t rate = angry ? 4 : 6;
        int16_t x = part_px[BP_CORE], y = part_py[BP_CORE];
        if (t % rate == 0) {
            bullet_fire(x, y + 4, spin, 24, (t & 8) ? BK_PINK : BK_BLUE);
            bullet_fire(x, y + 4, (uint8_t)(spin + 128), 24, (t & 8) ? BK_BLUE : BK_PINK);
            if (wings == 2)
                bullet_fire(x, y + 4, (uint8_t)(spin + 64), 22, BK_PINK);
            spin += 11;
        }
        if (angry && t % 90 == 45) {
            for (uint8_t k = 0; k < 12; k++)
                bullet_fire(x, y, (uint8_t)(k * 21 + (t & 15)), 20, BK_BLUE);
        }
    }
}

static void fight(void)
{
    uint16_t t = boss_t;
    boss_x = Q4(PF_W / 2) + (int16_t)(((int16_t)SIN8((uint8_t)(t >> 1)) * 20) >> 3);
    boss_y = Q4(66) + (int16_t)(((int16_t)SIN8((uint8_t)(t >> 2)) * 6) >> 3);
    attack();
}

static void frame(uint8_t i, mode5_csprite_t *s)
{
    if (i == BP_CANNON)
        s->xram_sprite_ptr = IMG(BOSS_CANNON) +
                             ((!part_dead[i] && boss_t % 180 > 140 && (boss_t & 8)) ? S1_BOSS_CANNON_FRAME_BYTES : 0);
}

const boss_def_t boss1 = {
    .portrait = IMG(PORTRAIT),
    .core_label = "CORE",
    .parts = parts,
    .n = BP_N,
    .core = BP_CORE,
    .time_limit = 5400, /* 90 seconds */
    .start = start,
    .enter = enter,
    .fight = fight,
    .frame = frame,
};
