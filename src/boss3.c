#include <rp6502.h>
#include "bg.h"
#include "boss.h"
#include "bosses.h"
#include "bullets.h"
#include "enemies.h"
#include "fx.h"
#include "game.h"
#include "player.h"
#include "sfx.h"
#include "util.h"
#include "xram.h"
#include "gen/stage3.h"

/*
 * Stage 3: LEVIATHAN. A battleship 416 pixels long, sailing bow first down
 * the screen. Its hull is plane 1's tile layer, so it costs no sprite time;
 * the guns, bridge and reactor are sprites riding on it, and the missile
 * bays are hull tiles that open to launch fighters. Destroyed parts leave
 * craters in the deck. The reactor at the stern is the core, so the ship
 * has to sail past before it can be finished.
 */

enum
{
    BP_BAY_L,
    BP_BAY_R,
    BP_TUR0,
    BP_TUR1,
    BP_TUR2,
    BP_TUR3,
    BP_TUR4,
    BP_TUR5,
    BP_GUN_A,
    BP_GUN_B,
    BP_BRIDGE,
    BP_REACTOR,
    BP_N
};

#define IMG(name) (XRAM_BANK + S3_##name)
#define HULL_MAP (XRAM_BANK + S3_HULL_MAP)
#define HULL_HALF_W (S3_HULL_W * 8)
#define HULL_HALF_H (S3_HULL_H * 8)

#define TUR(n) {S3_TUR##n##_OX, S3_TUR##n##_OY, 16, 16, 7, 7, 60, 3, IMG(BOSS_TURRET), 0, PK_TURRET, NO_PARENT, 1000}

static const part_t parts[BP_N] = {
    [BP_BAY_L] = {S3_BAY_L_OX, S3_BAY_L_OY, 16, 32, 7, 14, 120, 6, 0, 0, PK_HIDDEN, NO_PARENT, 2000},
    [BP_BAY_R] = {S3_BAY_R_OX, S3_BAY_R_OY, 16, 32, 7, 14, 120, 6, 0, 0, PK_HIDDEN, NO_PARENT, 2000},
    [BP_TUR0] = TUR(0),
    [BP_TUR1] = TUR(1),
    [BP_TUR2] = TUR(2),
    [BP_TUR3] = TUR(3),
    [BP_TUR4] = TUR(4),
    [BP_TUR5] = TUR(5),
    [BP_GUN_A] = {S3_GUN_A_OX, S3_GUN_A_OY, 32, 32, 12, 12, 200, 12, IMG(BOSS_GUN), 0, PK_TURRET, NO_PARENT, 3000},
    [BP_GUN_B] = {S3_GUN_B_OX, S3_GUN_B_OY, 32, 32, 12, 12, 200, 12, IMG(BOSS_GUN), 0, PK_TURRET, NO_PARENT, 3000},
    [BP_BRIDGE] = {S3_BRIDGE_OX, S3_BRIDGE_OY, 64, 64, 22, 24, 400, 20, IMG(BOSS_BRIDGE), 0, PK_SPRITE, NO_PARENT,
                   5000},
    [BP_REACTOR] = {S3_REACTOR_OX, S3_REACTOR_OY, 32, 32, 12, 12, 520, 26, IMG(BOSS_REACTOR), 0, PK_SPRITE, NO_PARENT,
                    10000},
};

/* Each part's socket on the hull, for craters: column, row. */
static const uint8_t socket[BP_N][2] = {
    [BP_BAY_L] = {S3_BAY_L_COL, S3_BAY_L_ROW},
    [BP_BAY_R] = {S3_BAY_R_COL, S3_BAY_R_ROW},
    [BP_TUR0] = {S3_TUR0_COL, S3_TUR0_ROW},
    [BP_TUR1] = {S3_TUR1_COL, S3_TUR1_ROW},
    [BP_TUR2] = {S3_TUR2_COL, S3_TUR2_ROW},
    [BP_TUR3] = {S3_TUR3_COL, S3_TUR3_ROW},
    [BP_TUR4] = {S3_TUR4_COL, S3_TUR4_ROW},
    [BP_TUR5] = {S3_TUR5_COL, S3_TUR5_ROW},
    [BP_GUN_A] = {S3_GUN_A_COL, S3_GUN_A_ROW},
    [BP_GUN_B] = {S3_GUN_B_COL, S3_GUN_B_ROW},
    [BP_BRIDGE] = {S3_BRIDGE_COL + 1, S3_BRIDGE_ROW + 1},
    [BP_REACTOR] = {S3_REACTOR_COL, S3_REACTOR_ROW},
};

/* Where the ship stops: the reactor in reach near the top of the screen. */
#define FINAL_Y Q4(76 - S3_REACTOR_OY)

static uint8_t bay_t[2];
static uint8_t spin;

static void tile(uint8_t col, uint8_t row, uint8_t t)
{
    xram0_poke8(HULL_MAP + row * S3_HULL_W + col, t);
}

static void crater2(uint8_t col, uint8_t row)
{
    tile(col, row, S3_T_CRATER_BIG0);
    tile((uint8_t)(col + 1), row, S3_T_CRATER_BIG1);
    tile(col, (uint8_t)(row + 1), S3_T_CRATER_BIG2);
    tile((uint8_t)(col + 1), (uint8_t)(row + 1), S3_T_CRATER_BIG3);
}

static void bay_tiles(uint8_t b, bool open)
{
    uint8_t i = (uint8_t)(BP_BAY_L + b);
    tile(socket[i][0], socket[i][1], open ? S3_T_BAYO0 : S3_T_BAY0);
    tile(socket[i][0], (uint8_t)(socket[i][1] + 1), open ? S3_T_BAYO1 : S3_T_BAY1);
}

static void place(void)
{
    bg_layer_move((boss_x >> 4) - HULL_HALF_W, (boss_y >> 4) - HULL_HALF_H);
}

static void start(void)
{
    /* The hull map came fresh from the ROM with the stage, so no craters yet. */
    bg_layer_show(HULL_MAP, S3_HULL_W, S3_HULL_H, XRAM_BANK + S3_HULL_TILES);
    boss_y = Q4(-HULL_HALF_H - 8);
    bay_t[0] = 0;
    bay_t[1] = 80;
    spin = 0;
    place();
}

static bool enter(void)
{
    boss_y += 12;
    return boss_y >= Q4(16);
}

/* A missile bay opens, launches a pair of fighters, and closes. */
static void bay(uint8_t b)
{
    uint8_t i = (uint8_t)(BP_BAY_L + b);
    if (part_dead[i])
        return;
    uint8_t c = bay_t[b]++;
    if (c == 120)
        bay_tiles(b, true);
    else if ((c == 144 || c == 156) && pl_state == PL_ALIVE) {
        int8_t slot = enemy_spawn(ET_DART, b ? COL_BLUE : COL_RED, part_px[i], part_py[i], 0);
        (void)slot;
        sfx_enemy(SFX_ENEMY_FIRE, SFX_PRI_EVENT);
    } else if (c == 170) {
        bay_tiles(b, false);
        bay_t[b] = 0;
    }
}

static void attack(void)
{
    uint16_t t = boss_t;
    bay(0);
    bay(1);
    if (pl_state != PL_ALIVE)
        return;
    bool angry = part_hp[BP_REACTOR] < parts[BP_REACTOR].hp / 2;

    /* Deck turrets: aimed blue shots, staggered down the ship. */
    for (uint8_t i = BP_TUR0; i <= BP_TUR5; i++) {
        if (part_dead[i])
            continue;
        uint8_t a = boss_aim(i);
        if ((uint8_t)(t + i * 13) % 75 == 0 && part_py[i] > 0 && part_py[i] < PF_H)
            bullet_fire(part_px[i], part_py[i], a, 30, BK_BLUE);
    }
    /* Main guns: slow, heavy fans of blue. */
    for (uint8_t i = BP_GUN_A; i <= BP_GUN_B; i++) {
        if (part_dead[i])
            continue;
        uint8_t a = boss_aim(i);
        if ((t + (i - BP_GUN_A) * 70) % 140 == 0 && part_py[i] > 0 && part_py[i] < PF_H) {
            bullet_fan(part_px[i] + (COS8(a) >> 3), part_py[i] + (SIN8(a) >> 3), a, 24, BK_BLUE_BIG, 5, 12);
            sfx_enemy(SFX_ENEMY_FIRE, SFX_PRI_EVENT);
        }
    }
    /* Bridge: wide pink sprays, there for the sword. */
    if (!part_dead[BP_BRIDGE] && t % 110 == 55 && part_py[BP_BRIDGE] > 0)
        bullet_fan(part_px[BP_BRIDGE], part_py[BP_BRIDGE] + 24, ANG_DOWN, 26, BK_PINK, 7, 10);
    /* Reactor: spirals once the bridge falls, or as time runs on. */
    if (part_dead[BP_BRIDGE] || t > 3600 || angry) {
        int16_t x = part_px[BP_REACTOR], y = part_py[BP_REACTOR];
        if (t % (angry ? 5 : 7) == 0) {
            bullet_fire(x, y, spin, 22, (t & 16) ? BK_PINK : BK_BLUE);
            bullet_fire(x, y, (uint8_t)(spin + 85), 22, BK_BLUE);
            bullet_fire(x, y, (uint8_t)(spin + 170), 22, (t & 16) ? BK_BLUE : BK_PINK);
            spin += 9;
        }
    }
}

static void fight(void)
{
    /* Sails on, slowly, until the reactor is in reach; sways a little. */
    if (boss_y < FINAL_Y)
        boss_y += 1;
    boss_x = Q4(PF_W / 2) + (int16_t)(((int16_t)SIN8((uint8_t)(boss_t >> 2)) * 6) >> 3);
    attack();
}

static void frame(uint8_t i, mode5_csprite_t *s)
{
    if (i == BP_REACTOR && !part_dead[i])
        s->xram_sprite_ptr = IMG(BOSS_REACTOR) + ((boss_t >> 3) & 1) * S3_BOSS_REACTOR_FRAME_BYTES;
}

static void destroyed(uint8_t i)
{
    uint8_t c = socket[i][0], r = socket[i][1];
    if (i == BP_BAY_L || i == BP_BAY_R) {
        tile(c, r, S3_T_CRATER_BAY0);
        tile(c, (uint8_t)(r + 1), S3_T_CRATER_BAY1);
    } else if (i == BP_GUN_A || i == BP_GUN_B || i == BP_BRIDGE || i == BP_REACTOR) {
        crater2(c, r);
    } else {
        tile(c, r, S3_T_CRATER);
    }
}

/* Burning from end to end as it goes down. */
static void dying(void)
{
    boss_y += 12;
    if ((boss_t & 7) == 0) {
        uint8_t c = (uint8_t)(1 + rng8() % (S3_HULL_W - 2));
        uint8_t r = (uint8_t)(1 + rng8() % (S3_HULL_H - 7));
        tile(c, r, S3_T_CRATER);
        fx_explode((boss_x >> 4) - HULL_HALF_W + c * 16 + 8, (boss_y >> 4) - HULL_HALF_H + r * 16 + 8, 1);
    }
}

static void finish(void)
{
    bg_layer_hide();
}

const boss_def_t boss3 = {
    .core_label = "REACTOR",
    .parts = parts,
    .n = BP_N,
    .core = BP_REACTOR,
    .time_limit = 7200, /* two minutes */
    .start = start,
    .enter = enter,
    .fight = fight,
    .place = place,
    .frame = frame,
    .destroyed = destroyed,
    .dying = dying,
    .finish = finish,
};
