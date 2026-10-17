#include "boss.h"
#include "bullets.h"
#include "fx.h"
#include "game.h"
#include "hud.h"
#include "player.h"
#include "score.h"
#include "sfx.h"
#include "sprites.h"
#include "targets.h"
#include "util.h"

/*
 * GAMMA CORE. Eight parts, each a sprite with its own hit points. Shooting
 * a part off adds its weight to the destruction percentage; killing the
 * core ends the fight. Take everything else apart first for 100%.
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

typedef struct
{
    int8_t ox, oy; /* center offset from the boss center */
    uint8_t w, h;
    uint8_t hw, hh;
    uint16_t hp;
    uint8_t weight;
    uint16_t img;
    uint8_t opt;
} part_t;

static const part_t parts[BP_N] = {
    [BP_CANNON] = {0, 44, 32, 32, 12, 10, 150, 12, XRAM_IMG_BOSS_CANNON, 0},
    [BP_WING_L] = {-48, -4, 32, 64, 13, 28, 180, 15, XRAM_IMG_BOSS_WING, 0},
    [BP_WING_R] = {48, -4, 32, 64, 13, 28, 180, 15, XRAM_IMG_BOSS_WING, MODE5_HFLIP},
    [BP_CORE] = {0, 0, 64, 64, 17, 17, 450, 30, XRAM_IMG_BOSS_CORE, 0},
    [BP_TUR0] = {-48, -16, 16, 16, 7, 7, 60, 7, XRAM_IMG_BOSS_TURRET, 0},
    [BP_TUR1] = {-48, 6, 16, 16, 7, 7, 60, 7, XRAM_IMG_BOSS_TURRET, 0},
    [BP_TUR2] = {47, -16, 16, 16, 7, 7, 60, 7, XRAM_IMG_BOSS_TURRET, 0},
    [BP_TUR3] = {47, 6, 16, 16, 7, 7, 60, 7, XRAM_IMG_BOSS_TURRET, 0},
};

enum
{
    BS_OFF,
    BS_ENTER,
    BS_FIGHT,
    BS_DYING,
    BS_ESCAPE,
    BS_DONE,
};

#define TIME_LIMIT 5400 /* 90 seconds */

uint8_t boss_destruction;
bool boss_escaped;

static uint8_t state;
static uint16_t t, state_t;
static int16_t bx, by; /* Q4 center */
static uint16_t hp[BP_N];
static uint8_t flash[BP_N], aim[BP_N];
static bool dead[BP_N];
static uint8_t spin;
static uint8_t shown_pct = 0xFF, shown_hp = 0xFF;

void boss_reset(void)
{
    state = BS_OFF;
    for (uint8_t i = 0; i < SPR_BOSS_N; i++)
        spr_hide(&spr[SPR_BOSS + i]);
    shown_pct = shown_hp = 0xFF;
}

void boss_start(void)
{
    state = BS_ENTER;
    t = state_t = 0;
    bx = Q4(PF_W / 2);
    by = Q4(-60);
    boss_destruction = 0;
    boss_escaped = false;
    spin = 0;
    for (uint8_t i = 0; i < BP_N; i++) {
        hp[i] = parts[i].hp;
        flash[i] = 0;
        dead[i] = false;
        aim[i] = 0;
        spr_setup(&spr[SPR_BOSS + i], parts[i].img, PAL_ADDR(PAL_BOSS), WH(parts[i].w, parts[i].h),
                  OPT4 | parts[i].opt);
    }
    shown_pct = shown_hp = 0xFF;
}

bool boss_done(void)
{
    return state == BS_DONE;
}

static int16_t part_x(uint8_t i)
{
    return (bx >> 4) + parts[i].ox;
}

static int16_t part_y(uint8_t i)
{
    return (by >> 4) + parts[i].oy;
}

static void destroy_part(uint8_t i, bool scored)
{
    if (dead[i])
        return;
    dead[i] = true;
    hp[i] = 0;
    int16_t x = part_x(i), y = part_y(i);
    fx_explode(x, y, parts[i].w >= 32);
    if (parts[i].h >= 64) {
        fx_explode(x, y - 20, 1);
        fx_explode(x, y + 20, 0);
    }
    sfx_enemy(SFX_BIG_EXPLODE, SFX_PRI_EVENT);
    if (scored) {
        boss_destruction += parts[i].weight;
        score_add(i == BP_CORE ? 20000 : 3000);
    }
    /* Turrets ride on the wings. */
    if (i == BP_WING_L) {
        destroy_part(BP_TUR0, scored);
        destroy_part(BP_TUR1, scored);
    } else if (i == BP_WING_R) {
        destroy_part(BP_TUR2, scored);
        destroy_part(BP_TUR3, scored);
    }
}

void boss_damage(uint8_t i, uint8_t dmg)
{
    if (state != BS_FIGHT || dead[i])
        return;
    flash[i] = 3;
    if ((t & 7) == 0)
        sfx_enemy(SFX_BOSS_HIT, SFX_PRI_FIRE);
    score_add(10);
    if (hp[i] <= dmg) {
        destroy_part(i, true);
        if (i == BP_CORE) {
            state = BS_DYING;
            state_t = 0;
            bullets_cancel_all();
        }
    } else {
        hp[i] -= dmg;
    }
}

static void attack(void)
{
    if (pl_state != PL_ALIVE)
        return;
    int16_t px = pl_x >> 4, py = pl_y >> 4;

    /* Turrets: aimed blue shots, staggered. */
    for (uint8_t i = BP_TUR0; i <= BP_TUR3; i++) {
        if (dead[i])
            continue;
        int16_t x = part_x(i), y = part_y(i);
        uint8_t a = angle_to(px - x, py - y);
        aim[i] = (uint8_t)(((uint8_t)(a - ANG_DOWN + 16)) >> 5);
        if ((uint8_t)(t + i * 17) % 64 == 0)
            bullet_fire(x, y, a, 30, BK_BLUE);
    }
    /* Wings: pink fans, alternating sides. */
    if (!dead[BP_WING_L] && t % 100 == 0)
        bullet_fan(part_x(BP_WING_L), part_y(BP_WING_L) + 28, ANG_DOWN + 8, 28, BK_PINK, 5, 12);
    if (!dead[BP_WING_R] && t % 100 == 50)
        bullet_fan(part_x(BP_WING_R), part_y(BP_WING_R) + 28, ANG_DOWN - 8, 28, BK_PINK, 5, 12);
    /* Cannon: charges, then lobs big pink orbs. Absorb them with the sword! */
    if (!dead[BP_CANNON]) {
        uint8_t c = (uint8_t)(t % 180);
        if (c == 0) {
            int16_t x = part_x(BP_CANNON), y = part_y(BP_CANNON) + 14;
            bullet_fan(x, y, angle_to(px - x, py - y), 22, BK_PINK_BIG, 3, 18);
            sfx_enemy(SFX_ENEMY_FIRE, SFX_PRI_EVENT);
        }
    }
    /* Core: spirals once it is exposed, or once time drags on. */
    uint8_t wings = (uint8_t)(dead[BP_WING_L] + dead[BP_WING_R]);
    bool angry = hp[BP_CORE] < parts[BP_CORE].hp / 2;
    if (wings || t > 1500 || angry) {
        uint8_t rate = angry ? 4 : 6;
        if (t % rate == 0) {
            int16_t x = part_x(BP_CORE), y = part_y(BP_CORE) + 4;
            bullet_fire(x, y, spin, 24, (t & 8) ? BK_PINK : BK_BLUE);
            bullet_fire(x, y, (uint8_t)(spin + 128), 24, (t & 8) ? BK_BLUE : BK_PINK);
            if (wings == 2)
                bullet_fire(x, y, (uint8_t)(spin + 64), 22, BK_PINK);
            spin += 11;
        }
        if (angry && t % 90 == 45) {
            int16_t x = part_x(BP_CORE), y = part_y(BP_CORE);
            for (uint8_t k = 0; k < 12; k++)
                bullet_fire(x, y, (uint8_t)(k * 21 + (t & 15)), 20, BK_BLUE);
        }
    }
}

static void draw(void)
{
    bool pulse = (t & 8) != 0;
    for (uint8_t i = 0; i < BP_N; i++) {
        mode5_csprite_t *s = &spr[SPR_BOSS + i];
        bool turret = i >= BP_TUR0;
        if (dead[i] && turret) {
            spr_hide(s);
            continue;
        }
        s->x_pos_px = PF_X + part_x(i) - (parts[i].w >> 1);
        s->y_pos_px = part_y(i) - (parts[i].h >> 1);
        uint8_t pal = PAL_BOSS;
        if (dead[i])
            pal = PAL_BOSS_WRECK;
        else if (flash[i]) {
            /* Big parts flash on alternate frames; full white is too much. */
            if (--flash[i] & 1)
                pal = PAL_FLASH;
        }
        s->palette_ptr = PAL_ADDR(pal);
        if (turret)
            s->xram_sprite_ptr = XRAM_IMG_BOSS_TURRET + aim[i] * BOSS_TURRET_FRAME_BYTES;
        else if (i == BP_CANNON)
            s->xram_sprite_ptr = XRAM_IMG_BOSS_CANNON +
                                 ((!dead[i] && t % 180 > 140 && pulse) ? BOSS_CANNON_FRAME_BYTES : 0);
    }
}

void boss_update(void)
{
    if (state == BS_OFF || state == BS_DONE)
        return;
    t++;
    state_t++;

    switch (state) {
    case BS_ENTER:
        by += 16;
        if (by >= Q4(66)) {
            state = BS_FIGHT;
            state_t = 0;
        }
        break;
    case BS_FIGHT:
        bx = Q4(PF_W / 2) + (int16_t)(((int16_t)SIN8((uint8_t)(t >> 1)) * 20) >> 3);
        by = Q4(66) + (int16_t)(((int16_t)SIN8((uint8_t)(t >> 2)) * 6) >> 3);
        attack();
        if (state_t > TIME_LIMIT) {
            state = BS_ESCAPE;
            boss_escaped = true;
            bullets_cancel_all();
        }
        break;
    case BS_DYING:
        /* Chain of explosions across whatever is left, then the big one. */
        if ((state_t & 5) == 0) {
            uint8_t i = (uint8_t)(rng8() & 3);
            fx_explode(part_x(i) + (int8_t)((rng8() & 31) - 16), part_y(i) + (int8_t)((rng8() & 31) - 16),
                       rng8() & 1);
            if ((state_t & 15) == 0)
                sfx_enemy(SFX_BIG_EXPLODE, SFX_PRI_TOP);
        }
        by += 4;
        if (state_t > 150) {
            for (uint8_t i = 0; i < BP_N; i++)
                destroy_part(i, false);
            for (uint8_t i = 0; i < SPR_BOSS_N; i++)
                spr_hide(&spr[SPR_BOSS + i]);
            state = BS_DONE;
            return;
        }
        break;
    case BS_ESCAPE:
        by -= 24;
        if (by < Q4(-120)) {
            for (uint8_t i = 0; i < SPR_BOSS_N; i++)
                spr_hide(&spr[SPR_BOSS + i]);
            state = BS_DONE;
            return;
        }
        break;
    }

    if (state == BS_FIGHT) {
        for (uint8_t i = 0; i < BP_N; i++) {
            if (!dead[i])
                tgt_add(part_x(i), part_y(i), parts[i].hw, parts[i].hh, (uint8_t)(TGT_BOSS + i));
        }
    }
    draw();
}

void boss_hud(bool force)
{
    if (state == BS_OFF) {
        return;
    }
    if (force || shown_pct == 0xFF) {
        hud_text(33, 14, "BOSS", HC_RED, HC_PANEL);
        hud_text(33, 15, "DESTROY", HC_GRAY, HC_PANEL);
    }
    if (boss_destruction != shown_pct || force) {
        shown_pct = boss_destruction;
        hud_u16(33, 16, boss_destruction, 3, boss_destruction >= 100 ? HC_YELLOW : HC_WHITE, HC_PANEL);
        hud_text(36, 16, "%", HC_GRAY, HC_PANEL);
    }
    uint8_t h = (uint8_t)(((uint16_t)hp[BP_CORE] * 14) / parts[BP_CORE].hp);
    if (h != shown_hp || force) {
        shown_hp = h;
        hud_text(33, 18, "CORE", HC_GRAY, HC_PANEL);
        hud_bar(33, 19, 7, h, HC_RED);
    }
}
