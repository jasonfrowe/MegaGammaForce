#include "bg.h"
#include "bullets.h"
#include "fx.h"
#include "game.h"
#include "hud.h"
#include "input.h"
#include "player.h"
#include "score.h"
#include "sfx.h"
#include "shots.h"
#include "sprites.h"
#include "targets.h"
#include "util.h"

int16_t pl_x, pl_y;
uint8_t pl_bx, pl_by;
uint8_t pl_state;
uint8_t pl_lives;
uint8_t pl_weapon;
uint8_t sword_gauge;
bool sword_active;
uint8_t sword_x0, sword_x1, sword_y0, sword_y1;
uint8_t hyper_timer;

static int8_t bank;
static uint8_t invuln;
static uint8_t dead_timer;
static uint8_t fire_cd;
static uint8_t sword_tick;
static uint8_t shown_weapon = 0xFF, shown_gauge = 0xFF, shown_lives = 0xFF, shown_level = 0xFF;
static uint8_t extend_timer, level_timer;

uint8_t wp_level[10];
uint8_t hit_weapon;
static uint16_t wp_xp[10];
/* Experience to reach each level. */
static const uint16_t level_xp[WP_LEVELS] = {0, 50, 200, 500, 1000};
#define LEVEL_ROW 19

/* A firing interval at the current weapon's level: x1 down to x1/2. */
static uint8_t cd(uint8_t w, uint8_t base)
{
    return (uint8_t)((base * (8 - wp_level[w]) + 7) >> 3);
}

#define LIVES_MAX 7 /* as many as the HUD shows */
#define EXTEND_ROW 18

/*
 * Homing missiles: while the button is held, a lock-on ring grows out from
 * the ship. Each target it sweeps over is locked, marked with a reticle,
 * and LOCK_DELAY frames later gets a missile of its own. After a full sweep
 * the ring starts again with fresh locks.
 */
#define LOCK_N (SPR_LOCK_N - SPR_LOCK_RING)
#define LOCK_DELAY 12 /* less 2 a level */
#define RING_MIN 10
#define RING_MAX 96
#define RING_GROW 3  /* pixels a frame, plus 1 a level */
#define RING_REST 16 /* frames at full size before the next sweep, less 2 a level */
static uint8_t ring_r, ring_rest;
static uint8_t lock_n;
static uint8_t lock_ref[LOCK_N], lock_timer[LOCK_N]; /* timer 0 = missile away */
static uint8_t lock_side;

static void lock_hide(void)
{
    for (uint8_t k = 0; k < SPR_LOCK_N; k++)
        spr_hide(&spr[SPR_LOCK + k]);
}

static void lock_reset(void)
{
    ring_r = 0;
    ring_rest = 0;
    lock_n = 0;
    lock_hide();
}

static bool locked(uint8_t ref)
{
    for (uint8_t k = 0; k < lock_n; k++)
        if (lock_ref[k] == ref)
            return true;
    return false;
}

static void missiles(void)
{
    /* Sweep the ring out, then rest, then start a new sweep. */
    if (ring_r == 0) {
        ring_r = RING_MIN;
        lock_n = 0;
    } else if (ring_r < RING_MAX) {
        ring_r += (uint8_t)(RING_GROW + wp_level[WP_MISSILE]);
    } else if (++ring_rest >= RING_REST - 2 * wp_level[WP_MISSILE]) {
        ring_rest = 0;
        ring_r = RING_MIN;
        lock_n = 0;
    }

    /* Lock anything inside the ring. Distance is max + min/2, near enough a circle. */
    for (uint8_t i = 0; i < tgt_n && lock_n < LOCK_N; i++) {
        uint8_t cx = (uint8_t)((tgt_x0[i] >> 1) + (tgt_x1[i] >> 1));
        uint8_t cy = (uint8_t)((tgt_y0[i] >> 1) + (tgt_y1[i] >> 1));
        uint8_t ax = absdiff8(cx, pl_bx), ay = absdiff8(cy, pl_by);
        uint16_t d = ax > ay ? (uint16_t)ax + (ay >> 1) : (uint16_t)ay + (ax >> 1);
        if (d <= ring_r && !locked(tgt_ref[i])) {
            lock_ref[lock_n] = tgt_ref[i];
            lock_timer[lock_n] = (uint8_t)(LOCK_DELAY - 2 * wp_level[WP_MISSILE]);
            lock_n++;
            sfx_player(SFX_LOCK, SFX_PRI_FIRE);
        }
    }

    /* Count down each lock, then launch, alternating sides. */
    for (uint8_t k = 0; k < lock_n; k++) {
        if (lock_timer[k] && --lock_timer[k] == 0) {
            lock_side ^= 1;
            shot_missile((uint8_t)(pl_bx + (lock_side ? 8 : -8)), pl_by, lock_side ? ANG_UP + 48 : ANG_UP - 48,
                         lock_ref[k]);
            sfx_player(SFX_HOMING, SFX_PRI_FIRE);
        }
    }

    /* The ring: dots turning around the ship. */
    int16_t sx = PF_X + (pl_bx - BS_X), sy = pl_by - BS_Y;
    uint8_t spin = (uint8_t)(frame_count << 1);
    for (uint8_t k = 0; k < SPR_LOCK_RING; k++) {
        mode5_csprite_t *s = &spr[SPR_LOCK + k];
        uint8_t a = (uint8_t)(spin + k * 32);
        spr_setup(s, XRAM_IMG_SHOT8 + (SHOT8_HOMING + 1) * SHOT8_FRAME_BYTES, PAL_ADDR(PAL_SHOTS), WH(8, 8), OPT4);
        s->x_pos_px = sx + (((int16_t)COS8(a) * ring_r) >> 7) - 4;
        s->y_pos_px = sy + (((int16_t)SIN8(a) * ring_r) >> 7) - 4;
    }
    /* Reticles on the locked targets that are still there. */
    for (uint8_t k = 0; k < LOCK_N; k++) {
        mode5_csprite_t *s = &spr[SPR_LOCK + SPR_LOCK_RING + k];
        int8_t t = -1;
        if (k < lock_n)
            for (uint8_t i = 0; i < tgt_n; i++)
                if (tgt_ref[i] == lock_ref[k])
                    t = (int8_t)i;
        if (t < 0) {
            spr_hide(s);
            continue;
        }
        uint8_t cx = (uint8_t)((tgt_x0[(uint8_t)t] >> 1) + (tgt_x1[(uint8_t)t] >> 1));
        uint8_t cy = (uint8_t)((tgt_y0[(uint8_t)t] >> 1) + (tgt_y1[(uint8_t)t] >> 1));
        spr_setup(s, XRAM_IMG_LOCK + ((frame_count >> 2) & 1) * LOCK_FRAME_BYTES, PAL_ADDR(PAL_SHOTS), WH(16, 16), OPT4);
        s->x_pos_px = (int16_t)cx + (PF_X - BS_X - 8);
        s->y_pos_px = (int16_t)cy - (BS_Y + 8);
    }
}

#define X_MIN Q4(10)
#define X_MAX Q4(PF_W - 10)
#define Y_MIN Q4(16)
#define Y_MAX Q4(PF_H - 14)

static void beam_hide(void)
{
    for (uint8_t k = 0; k < SPR_BEAM_N; k++)
        spr_hide(&spr[SPR_BEAM + k]);
}

void player_hide(void)
{
    beam_hide();
    lock_reset();
    spr_hide(&spr[SPR_PLAYER]);
    spr_hide(&spr[SPR_FLAME]);
    spr_hide(&spr[SPR_FLAME + 1]);
    spr_hide(&spr[SPR_SWORD]);
}

void player_respawn(void)
{
    pl_x = Q4(PF_W / 2);
    pl_y = Q4(PF_H - 30);
    pl_state = PL_ALIVE;
    invuln = 120;
    bank = 0;
    fire_cd = 0;
    sword_active = false;
}

void player_reset(void)
{
    pl_lives = 3;
    sword_gauge = 0;
    hyper_timer = 0;
    pl_weapon = WP_VULCAN;
    for (uint8_t w = 0; w < 10; w++) {
        wp_level[w] = 0;
        wp_xp[w] = 0;
    }
    level_timer = 0;
    shown_weapon = shown_gauge = shown_lives = shown_level = 0xFF;
    extend_timer = 0;
    player_respawn();
    invuln = 0;
}

bool player_vulnerable(void)
{
    return pl_state == PL_ALIVE && invuln == 0 && hyper_timer == 0;
}

void player_hit(void)
{
    if (!player_vulnerable())
        return;
    fx_explode(pl_x >> 4, pl_y >> 4, 1);
    fx_explode((pl_x >> 4) - 6, (pl_y >> 4) + 4, 0);
    fx_explode((pl_x >> 4) + 6, (pl_y >> 4) - 4, 0);
    sfx_player(SFX_PLAYER_DIE, SFX_PRI_TOP);
    pl_state = PL_DEAD;
    dead_timer = 90;
    if (pl_lives)
        pl_lives--;
    sword_active = false;
    sword_gauge = 0; /* dying loses the Hyper Sword charge */
    chain_break();   /* and the chain */
    player_hide();
}

void sword_absorb(void)
{
    if (sword_gauge < SWORD_GAUGE_MAX)
        sword_gauge++;
    score_add(10);
}

/* Damage every target overlapping the sword box. */
static void sword_strike(uint8_t dmg)
{
    hit_weapon = hyper_timer ? WP_HYPER : WP_SWORD;
    for (uint8_t i = 0; i < tgt_n; i++) {
        if (tgt_x1[i] > sword_x0 && tgt_x0[i] < sword_x1 && tgt_y1[i] > sword_y0 && tgt_y0[i] < sword_y1)
            tgt_damage(i, dmg);
    }
}

static void fire(uint8_t w)
{
    if (w != WP_MISSILE && (ring_r || lock_n))
        lock_reset(); /* letting go cancels the sweep and any pending locks */
    uint8_t nx = pl_bx, ny = (uint8_t)(pl_by - 10);
    if (fire_cd)
        fire_cd--;

    switch (w) {
    case WP_VULCAN:
        if (fire_cd == 0 && shots_count(SK_VULCAN) < 8 + 2 * wp_level[w]) {
            shot_fire(SK_VULCAN, nx, (uint8_t)(ny - 4), ANG_UP, 128);
            fire_cd = cd(w, 4);
            sfx_player(SFX_VULCAN, SFX_PRI_FIRE);
        }
        break;
    case WP_MISSILE:
        missiles();
        break;
    case WP_HOMING:
        if (fire_cd == 0 && shots_count(SK_HOMING) < 8 + 2 * wp_level[w]) {
            /* One dart every 8 frames, alternating sides: 7.5 damage/s at LV1. */
            static bool side;
            side = !side;
            if (side)
                shot_fire(SK_HOMING, (uint8_t)(pl_bx - 10), pl_by, ANG_UP - 40, 60);
            else
                shot_fire(SK_HOMING, (uint8_t)(pl_bx + 10), pl_by, ANG_UP + 40, 60);
            fire_cd = cd(w, 8);
            sfx_player(SFX_HOMING, SFX_PRI_FIRE);
        }
        break;
    case WP_SPREAD:
        if (fire_cd == 0) {
            /* Five orbs; seven from LV3. Its rate only climbs to LV2's, and
             * from LV4 the orbs hit harder instead (shots.c). */
            static const uint8_t spread_cd[WP_LEVELS] = {10, 9, 9, 8, 8};
            int8_t step = wp_level[w] >= 2 ? 8 : 12;
            for (int8_t a = -24; a <= 24; a += step)
                shot_fire(SK_SPREAD, nx, ny, (uint8_t)(ANG_UP + a), 88);
            fire_cd = spread_cd[wp_level[w]];
            sfx_player(SFX_SPREAD, SFX_PRI_FIRE);
        }
        break;
    case WP_WIDE:
        if (fire_cd == 0) {
            shot_fire(SK_VULCAN, nx, (uint8_t)(ny - 4), ANG_UP, 128);
            shot_fire(SK_BACK, (uint8_t)(pl_bx - 6), pl_by, ANG_DOWN + 36, 80);
            shot_fire(SK_BACK, (uint8_t)(pl_bx + 6), pl_by, ANG_DOWN - 36, 80);
            shot_fire(SK_BACK, (uint8_t)(pl_bx - 8), pl_by, 128 + 12, 80);
            shot_fire(SK_BACK, (uint8_t)(pl_bx + 8), pl_by, (uint8_t)(0 - 12), 80);
            fire_cd = cd(w, 8);
            sfx_player(SFX_SPREAD, SFX_PRI_FIRE);
        }
        break;
    case WP_PLASMA: {
        /* A crackling link to the nearest target in front of the ship. */
        int8_t t = tgt_nearest(nx, ny);
        if (t >= 0 && tgt_y1[(uint8_t)t] < pl_by) {
            uint8_t tx = (uint8_t)((tgt_x0[(uint8_t)t] >> 1) + (tgt_x1[(uint8_t)t] >> 1));
            uint8_t ty = (uint8_t)((tgt_y0[(uint8_t)t] >> 1) + (tgt_y1[(uint8_t)t] >> 1));
            int16_t dx = (int16_t)tx - nx, dy = (int16_t)ty - ny;
            for (uint8_t k = 1; k <= 5; k++) {
                int16_t f = (int16_t)(k * 13); /* k/5 in 64ths */
                uint8_t jx = (uint8_t)((rng8() & 7) - 3);
                shot_spark((uint8_t)(nx + ((dx * f) >> 6) + (int8_t)jx), (uint8_t)(ny + ((dy * f) >> 6)));
            }
            /* Weakest weapon: it locks on by itself, 1 every 8 frames = 7.5/s at LV1. */
            if (fire_cd == 0) {
                hit_weapon = WP_PLASMA;
                tgt_damage((uint8_t)t, 1);
                fire_cd = cd(w, 8);
                sfx_player(SFX_PLASMA, SFX_PRI_FIRE);
            }
        } else {
            for (uint8_t k = 1; k <= 3; k++)
                shot_spark((uint8_t)(nx + (rng8() & 3) - 1), (uint8_t)(ny - k * 9));
        }
        break;
    }
    default:
        break;
    }
}

static void update_sword(uint8_t w, int16_t sx, int16_t sy)
{
    mode5_csprite_t *s = &spr[SPR_SWORD];
    bool hyper = hyper_timer != 0;
    sword_active = hyper || w == WP_SWORD;
    if (!hyper)
        beam_hide();
    if (!sword_active) {
        spr_hide(s);
        return;
    }
    sword_tick++;
    uint8_t f = (uint8_t)((sword_tick >> 2) & 3);
    if (hyper) {
        /* Doubled: 96 x 64 of blade. */
        spr_setup(s, XRAM_IMG_SWORD + f * SWORD_FRAME_BYTES, PAL_ADDR((sword_tick & 4) ? PAL_SHOTS : PAL_FLASH), WH(48, 32),
                  OPT4 | MODE5_HDOUBLE | MODE5_VDOUBLE);
        s->x_pos_px = sx - 48;
        s->y_pos_px = sy - 70;
        sword_x0 = pl_bx > 48 ? (uint8_t)(pl_bx - 48) : 0;
        sword_x1 = pl_bx < 207 ? (uint8_t)(pl_bx + 48) : 255;
        sword_y0 = pl_by > 72 ? (uint8_t)(pl_by - 72) : 0;
        sword_y1 = (uint8_t)(pl_by - 4);
        if ((sword_tick & 3) == 0)
            sword_strike(3); /* 45/s */
        /* The blade's energy also lances up the screen as a beam. */
        int16_t top = sy - 16 - 128;
        for (uint8_t k = 0; k < SPR_BEAM_N; k++) {
            mode5_csprite_t *b = &spr[SPR_BEAM + k];
            spr_setup(b, XRAM_IMG_BEAM + ((sword_tick >> 2) & 1) * BEAM_FRAME_BYTES, PAL_ADDR(PAL_SHOTS), WH(32, 64),
                      OPT4 | MODE5_VDOUBLE);
            b->x_pos_px = sx - 16;
            b->y_pos_px = top - 128 * k;
        }
        if ((sword_tick & 3) == 2) { /* beam 2 every 4 frames, 30/s */
            uint8_t x0 = (uint8_t)(pl_bx - 14), x1 = (uint8_t)(pl_bx + 14);
            hit_weapon = WP_HYPER;
            for (uint8_t i = 0; i < tgt_n; i++)
                if (tgt_x1[i] > x0 && tgt_x0[i] < x1 && tgt_y0[i] < pl_by)
                    tgt_damage(i, 2);
        }
    } else {
        spr_setup(s, XRAM_IMG_SWORD + f * SWORD_FRAME_BYTES, PAL_ADDR(PAL_SHOTS), WH(48, 32), OPT4);
        s->x_pos_px = sx - 24;
        s->y_pos_px = sy - 34;
        sword_x0 = pl_bx > 24 ? (uint8_t)(pl_bx - 24) : 0;
        sword_x1 = pl_bx < 231 ? (uint8_t)(pl_bx + 24) : 255;
        sword_y0 = pl_by > 36 ? (uint8_t)(pl_by - 36) : 0;
        sword_y1 = (uint8_t)(pl_by - 4);
        /* 3 every 8 frames = 22/s at LV1, faster and harder each level;
         * it also absorbs pink bullets. fire() counts fire_cd down. */
        if (fire_cd == 0) {
            sword_strike((uint8_t)(3 + (wp_level[WP_SWORD] >> 1)));
            fire_cd = cd(WP_SWORD, 8);
        }
        if ((sword_tick & 15) == 0)
            sfx_player(SFX_SWORD, SFX_PRI_FIRE);
    }
}

void weapon_xp(uint8_t xp)
{
    uint8_t w = hit_weapon;
    if (w == WP_NONE || w >= WP_HYPER || w == WP_ALL)
        return;
    uint8_t lv = wp_level[w];
    if (lv >= WP_LEVELS - 1)
        return;
    if (w == WP_VULCAN)
        xp = (uint8_t)((xp + 1) >> 1); /* the strongest gun learns slowest */
    wp_xp[w] += xp; /* stops at the top level, well short of overflowing */
    if (wp_xp[w] >= level_xp[lv + 1]) {
        wp_level[w] = ++lv;
        /* "SPREAD LV3" in the playfield for a moment. */
        char msg[] = "LEVEL UP! LV ";
        msg[12] = (char)('1' + lv);
        hud_clear_row(LEVEL_ROW);
        hud_center(LEVEL_ROW, msg, HC_GREEN);
        level_timer = 90;
        sfx_player(SFX_LEVELUP, SFX_PRI_EVENT);
    }
}

/* Extra lives from the score: one each 100,000 points. */
static void extends(void)
{
    if (extend_timer && --extend_timer == 0)
        hud_clear_row(EXTEND_ROW);
    if (level_timer && --level_timer == 0)
        hud_clear_row(LEVEL_ROW);
    if (!score_extends || (pl_state == PL_DEAD && pl_lives == 0))
        return;
    score_extends--;
    if (pl_lives < LIVES_MAX)
        pl_lives++;
    hud_clear_row(EXTEND_ROW);
    hud_text(16, EXTEND_ROW, "EXTEND!", HC_YELLOW, HC_CLEAR);
    extend_timer = 120;
    sfx_player(SFX_EXTEND, SFX_PRI_TOP);
}

void player_update(bool controls)
{
    extends();
    if (pl_state == PL_DEAD) {
        if (dead_timer && --dead_timer == 0 && pl_lives)
            player_respawn();
        return;
    }

    uint16_t held = controls ? in_held : 0;
    uint16_t pressed = controls ? in_pressed : 0;
    uint8_t w = (uint8_t)((held >> 4) & 7);
    if (w == WP_ALL)
        w = WP_VULCAN;
    if (held & IN_SWORD)
        w = WP_SWORD;

    /* Hyper sword: its own button, and only with a full gauge. Holding the
     * sword while the gauge fills never sets it off. */
    if ((pressed & IN_HYPER) && sword_gauge >= SWORD_GAUGE_MAX && hyper_timer == 0) {
        hyper_timer = 150;
        sword_gauge = 0;
        uint8_t n = bullets_cancel_all();
        score_add((uint16_t)n * 20);
        sfx_player(SFX_HYPER, SFX_PRI_TOP);
    }
    if (hyper_timer)
        hyper_timer--;

    /* Movement: slower while firing, slower still with the sword out. */
    int16_t sp = w ? (sword_active ? 18 : 24) : 32;
    int16_t dx = 0, dy = 0;
    if (held & IN_LEFT)
        dx = -sp;
    if (held & IN_RIGHT)
        dx = sp;
    if (held & IN_UP)
        dy = -sp;
    if (held & IN_DOWN)
        dy = sp;
    if (dx && dy) {
        dx = (int16_t)((dx * 3) >> 2);
        dy = (int16_t)((dy * 3) >> 2);
    }
    pl_x += dx;
    pl_y += dy;
    if (pl_x < X_MIN)
        pl_x = X_MIN;
    if (pl_x > X_MAX)
        pl_x = X_MAX;
    if (pl_y < Y_MIN)
        pl_y = Y_MIN;
    if (pl_y > Y_MAX)
        pl_y = Y_MAX;

    if (dx < 0 && bank > -10)
        bank--;
    else if (dx > 0 && bank < 10)
        bank++;
    else if (dx == 0 && bank)
        bank += bank > 0 ? -1 : 1;

    int16_t gx = pl_x >> 4, gy = pl_y >> 4;
    pl_bx = (uint8_t)(gx + BS_X);
    pl_by = (uint8_t)(gy + BS_Y);

    /* Flying into a tower is fatal. */
    if (bg_sky_hit(gx - 4, gy - 4) || bg_sky_hit(gx + 4, gy - 4) || bg_sky_hit(gx - 4, gy + 4) ||
        bg_sky_hit(gx + 4, gy + 4)) {
        player_hit();
        if (pl_state != PL_ALIVE)
            return;
    }
    if (invuln)
        invuln--;

    if (w)
        pl_weapon = w;
    fire(w);

    /* Sprites. */
    int16_t sx = PF_X + gx - 12, sy = gy - 12;
    uint8_t ab = (uint8_t)(bank < 0 ? -bank : bank);
    uint8_t f = ab < 4 ? 0 : (ab < 9 ? 1 : 2);
    uint8_t flip = bank < 0 ? MODE5_HFLIP : 0;
    bool blink = invuln && (invuln & 4);
    mode5_csprite_t *s = &spr[SPR_PLAYER];
    spr_setup(s, XRAM_IMG_PLAYER + f * PLAYER_FRAME_BYTES, PAL_ADDR(hyper_timer && (frame_count & 2) ? PAL_FLASH : PAL_PLAYER),
              WH(24, 24), OPT4 | flip);
    s->x_pos_px = sx;
    s->y_pos_px = blink ? SPR_HIDDEN_Y : sy;

    static const int8_t flame_l[3] = {4, 5, 6}, flame_r[3] = {11, 10, 9};
    uint8_t ff = (uint8_t)((frame_count >> 1) & 3);
    for (uint8_t k = 0; k < 2; k++) {
        mode5_csprite_t *fl = &spr[SPR_FLAME + k];
        spr_setup(fl, XRAM_IMG_FLAME + ((ff + k * 2) & 3) * FLAME_FRAME_BYTES, PAL_ADDR(PAL_PLAYER), WH(8, 8), OPT4);
        fl->x_pos_px = sx + (k ? flame_r[f] : flame_l[f]);
        fl->y_pos_px = blink ? SPR_HIDDEN_Y : sy + 21;
    }
    if (bank < 0) {
        /* Mirrored frames: swap the flame offsets. */
        spr[SPR_FLAME].x_pos_px = sx + 23 - 7 - flame_r[f];
        spr[SPR_FLAME + 1].x_pos_px = sx + 23 - 7 - flame_l[f];
    }

    update_sword(w, sx + 12, sy + 12);
}

static const char *const weapon_name[10][2] = {
    {"", ""},
    {"VULCAN", ""},
    {"HOMING", ""},
    {"HOMING", "PLASMA"},
    {"SPREAD", ""},
    {"WIDE", "SHOT"},
    {"HOMING", "MISSILE"},
    {"VULCAN", ""},
    {"RADIANT", "SWORD"},
    {"HYPER", "SWORD"},
};

void player_hud(bool force)
{
    uint8_t w = hyper_timer ? WP_HYPER : pl_weapon;
    if (w != shown_weapon || force) {
        shown_weapon = w;
        hud_fill(33, 3, 7, ' ', HC_WHITE, HC_PANEL);
        hud_fill(33, 4, 7, ' ', HC_WHITE, HC_PANEL);
        hud_text(33, 3, weapon_name[w][0], HC_WHITE, HC_PANEL);
        hud_text(33, 4, weapon_name[w][1], HC_WHITE, HC_PANEL);
    }
    /* The weapon's level, and its progress to the next one. */
    uint8_t lw = w == WP_ALL ? WP_VULCAN : w;
    uint8_t shown = 0xFE;
    uint8_t halves = 0;
    if (lw && lw < WP_HYPER) {
        uint8_t lv = wp_level[lw];
        if (lv < WP_LEVELS - 1) {
            uint16_t lo = level_xp[lv], hi = level_xp[lv + 1];
            halves = (uint8_t)((uint16_t)(wp_xp[lw] - lo) * 6 / (hi - lo));
        }
        shown = (uint8_t)((lw << 4) | (lv << 3) | halves);
    }
    if (shown != shown_level || force) {
        shown_level = shown;
        hud_fill(33, 5, 7, ' ', HC_WHITE, HC_PANEL);
        if (shown != 0xFE) {
            uint8_t lv = wp_level[lw];
            char t[] = "LV1";
            t[2] = (char)('1' + lv);
            hud_text(33, 5, t, lv ? HC_GREEN : HC_GRAY, HC_PANEL);
            if (lv >= WP_LEVELS - 1)
                hud_text(37, 5, "MAX", HC_YELLOW, HC_PANEL);
            else
                hud_bar(37, 5, 3, halves, HC_GREEN);
        }
    }
    uint8_t g = sword_gauge;
    if (g != shown_gauge || force) {
        shown_gauge = g;
        uint8_t halves = (uint8_t)((uint16_t)g * 14 / SWORD_GAUGE_MAX);
        hud_bar(33, 7, 7, halves, g >= SWORD_GAUGE_MAX ? ((frame_count & 8) ? HC_WHITE : HC_CYAN) : HC_VIOLET);
        hud_text(33, 8, g >= SWORD_GAUGE_MAX ? "HYPER!" : "      ", HC_YELLOW, HC_PANEL);
    }
    if (g >= SWORD_GAUGE_MAX && (frame_count & 7) == 0)
        shown_gauge = 0xFF; /* keep the full gauge flashing */
    if (pl_lives != shown_lives || force) {
        shown_lives = pl_lives;
        hud_fill(33, 11, 7, ' ', HC_WHITE, HC_PANEL);
        for (uint8_t i = 0; i < pl_lives && i < 7; i++)
            hud_fill((uint8_t)(33 + i), 11, 1, 0x1E, HC_CYAN, HC_PANEL);
    }
}
