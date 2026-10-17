#include "boss.h"
#include "bosses.h"
#include "bullets.h"
#include "game.h"
#include "player.h"
#include "sfx.h"
#include "util.h"
#include "xram.h"
#include "gen/stage2.h"

/*
 * Stage 2: SPIRE WARDEN. A defence core with three pods orbiting it, a twin
 * cannon slung beneath and two shoulder turrets. The pods spin faster as
 * they fall, and the core opens up once two are gone.
 */

enum
{
    BP_CANNON,
    BP_CORE,
    BP_POD0,
    BP_POD1,
    BP_POD2,
    BP_TUR_L,
    BP_TUR_R,
    BP_N
};

#define IMG(name) (XRAM_BANK + S2_##name)

static const part_t parts[BP_N] = {
    [BP_CANNON] = {0, 42, 32, 32, 11, 10, 160, 20, IMG(BOSS_CANNON), 0, PK_SPRITE, NO_PARENT, 2000},
    [BP_CORE] = {0, 0, 64, 64, 18, 18, 500, 30, IMG(BOSS_CORE), 0, PK_SPRITE, NO_PARENT, 10000},
    [BP_POD0] = {0, 0, 32, 32, 11, 11, 120, 12, IMG(BOSS_POD), 0, PK_SPRITE, NO_PARENT, 2000},
    [BP_POD1] = {0, 0, 32, 32, 11, 11, 120, 12, IMG(BOSS_POD), 0, PK_SPRITE, NO_PARENT, 2000},
    [BP_POD2] = {0, 0, 32, 32, 11, 11, 120, 12, IMG(BOSS_POD), 0, PK_SPRITE, NO_PARENT, 2000},
    [BP_TUR_L] = {-26, -20, 16, 16, 7, 7, 50, 7, IMG(BOSS_TURRET), 0, PK_TURRET, NO_PARENT, 2000},
    [BP_TUR_R] = {26, -20, 16, 16, 7, 7, 50, 7, IMG(BOSS_TURRET), 0, PK_TURRET, NO_PARENT, 2000},
};

static uint8_t orbit, spin, sweep_t;
static int8_t sweep_dir;

static uint8_t pods_dead(void)
{
    return (uint8_t)(part_dead[BP_POD0] + part_dead[BP_POD1] + part_dead[BP_POD2]);
}

static void start(void)
{
    orbit = spin = sweep_t = 0;
    sweep_dir = 1;
}

static bool enter(void)
{
    orbit += 2;
    boss_y += 14;
    return boss_y >= Q4(72);
}

static void attack(void)
{
    if (pl_state != PL_ALIVE)
        return;
    uint16_t t = boss_t;
    bool angry = part_hp[BP_CORE] < parts[BP_CORE].hp / 2;

    /* Pods: aimed pink triplets, one pod after another. */
    uint8_t rate = angry ? 60 : 90;
    for (uint8_t i = BP_POD0; i <= BP_POD2; i++) {
        if (part_dead[i])
            continue;
        if ((uint16_t)(t + (i - BP_POD0) * 30) % rate == 0) {
            int16_t x = part_px[i], y = part_py[i];
            bullet_fan(x, y, angle_to((pl_x >> 4) - x, (pl_y >> 4) - y), 30, BK_PINK, 3, 12);
        }
    }
    /* Shoulder turrets: aimed blue shots. */
    for (uint8_t i = BP_TUR_L; i <= BP_TUR_R; i++) {
        if (part_dead[i])
            continue;
        uint8_t a = boss_aim(i);
        if ((uint8_t)(t + i * 35) % 70 == 0)
            bullet_fire(part_px[i], part_py[i], a, 32, BK_BLUE);
    }
    /* Cannon: charges, then sweeps a stream of needles across the screen. */
    if (!part_dead[BP_CANNON]) {
        uint8_t c = (uint8_t)(t % 200);
        if (c == 0) {
            sweep_t = 48;
            sweep_dir = (int8_t)-sweep_dir;
            sfx_enemy(SFX_ENEMY_FIRE, SFX_PRI_EVENT);
        }
        if (sweep_t) {
            sweep_t--;
            if ((sweep_t & 3) == 0) {
                int8_t off = (int8_t)((int8_t)(24 - sweep_t) * sweep_dir);
                uint8_t a = (uint8_t)(ANG_DOWN + off);
                bullet_fire(part_px[BP_CANNON] - 5, part_py[BP_CANNON] + 14, a, 40, BK_NEEDLE);
                bullet_fire(part_px[BP_CANNON] + 5, part_py[BP_CANNON] + 14, a, 40, BK_NEEDLE);
            }
        }
    }
    /* Core: spinning rings once it is exposed or the fight drags on. */
    if (pods_dead() >= 2 || t > 1500 || angry) {
        if (t % (angry ? 36 : 50) == 0) {
            int16_t x = part_px[BP_CORE], y = part_py[BP_CORE];
            for (uint8_t k = 0; k < 16; k++)
                bullet_fire(x, y, (uint8_t)(k * 16 + spin), 22, (k & 1) ? BK_PINK : BK_BLUE);
            spin += 7;
        }
        if (angry && t % 120 == 60) {
            int16_t x = part_px[BP_CORE], y = part_py[BP_CORE] + 10;
            bullet_fan(x, y, angle_to((pl_x >> 4) - x, (pl_y >> 4) - y), 20, BK_PINK_BIG, 3, 20);
        }
    }
}

static void fight(void)
{
    uint16_t t = boss_t;
    boss_x = Q4(PF_W / 2) + (int16_t)(((int16_t)SIN8((uint8_t)(t >> 1)) * 24) >> 3);
    boss_y = Q4(72) + (int16_t)(((int16_t)SIN8((uint8_t)(t >> 2)) * 8) >> 3);
    orbit += (uint8_t)(1 + pods_dead());
    attack();
}

/* The pods circle the core on a flattened orbit, a third of a turn apart. */
static void place(void)
{
    int16_t cx = part_px[BP_CORE], cy = part_py[BP_CORE];
    for (uint8_t k = 0; k < 3; k++) {
        uint8_t a = (uint8_t)(orbit + k * 85);
        part_px[BP_POD0 + k] = cx + (int16_t)(((int16_t)COS8(a) * 58) >> 7);
        part_py[BP_POD0 + k] = cy + (int16_t)(((int16_t)SIN8(a) * 40) >> 7);
    }
}

static void frame(uint8_t i, mode5_csprite_t *s)
{
    if (i >= BP_POD0 && i <= BP_POD2)
        s->xram_sprite_ptr = IMG(BOSS_POD) + (((boss_t >> 3) + i) & 1) * S2_BOSS_POD_FRAME_BYTES;
    else if (i == BP_CANNON)
        s->xram_sprite_ptr = IMG(BOSS_CANNON) +
                             ((!part_dead[i] && boss_t % 200 > 170 && (boss_t & 4)) ? S2_BOSS_CANNON_FRAME_BYTES : 0);
}

const boss_def_t boss2 = {
    .core_label = "CORE",
    .parts = parts,
    .n = BP_N,
    .core = BP_CORE,
    .time_limit = 5400,
    .start = start,
    .enter = enter,
    .fight = fight,
    .place = place,
    .frame = frame,
};
