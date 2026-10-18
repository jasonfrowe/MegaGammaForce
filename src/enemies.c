#include "bg.h"
#include "bullets.h"
#include "enemies.h"
#include "fx.h"
#include "game.h"
#include "player.h"
#include "score.h"
#include "sfx.h"
#include "sprites.h"
#include "stage.h"
#include "targets.h"
#include "util.h"
#include "gen/stage4.h"

typedef struct
{
    uint8_t hp;
    uint16_t score;
    uint8_t hw, hh;  /* hitbox half sizes */
    uint8_t w, h;    /* sprite size */
    uint16_t img;
    uint16_t frame_bytes;
    uint8_t frames;
} etype_t;

static const etype_t etypes[] = {
    [ET_DART] = {2, 100, 6, 6, 16, 16, XRAM_IMG_ENEMY16 + ENEMY16_DART * ENEMY16_FRAME_BYTES, ENEMY16_FRAME_BYTES, 2},
    [ET_SAUCER] = {4, 200, 7, 6, 16, 16, XRAM_IMG_ENEMY16 + ENEMY16_SAUCER * ENEMY16_FRAME_BYTES, ENEMY16_FRAME_BYTES, 4},
    [ET_DRONE] = {10, 300, 7, 7, 16, 16, XRAM_IMG_ENEMY16 + ENEMY16_DRONE * ENEMY16_FRAME_BYTES, ENEMY16_FRAME_BYTES, 2},
    [ET_GUNSHIP] = {60, 2000, 14, 13, 32, 32, XRAM_IMG_ENEMY32 + ENEMY32_GUNSHIP * ENEMY32_FRAME_BYTES, ENEMY32_FRAME_BYTES, 2},
    [ET_TURRET] = {8, 300, 7, 7, 16, 16, XRAM_IMG_ENEMY16 + ENEMY16_TURRET * ENEMY16_FRAME_BYTES, ENEMY16_FRAME_BYTES, 1},
    [ET_TANK] = {10, 400, 7, 6, 16, 16, XRAM_IMG_ENEMY16 + ENEMY16_TANK * ENEMY16_FRAME_BYTES, ENEMY16_FRAME_BYTES, 1},
    [ET_WASP] = {3, 150, 6, 6, 16, 16, XRAM_IMG_ENEMY16 + ENEMY16_WASP * ENEMY16_FRAME_BYTES, ENEMY16_FRAME_BYTES, 2},
    [ET_GUNBOAT] = {40, 1500, 8, 14, 32, 32, XRAM_IMG_ENEMY32 + ENEMY32_GUNBOAT * ENEMY32_FRAME_BYTES, ENEMY32_FRAME_BYTES,
                    2},
    /* Stage art: only stage 4 has trains, so its file holds them. */
    [ET_TRAIN] = {24, 800, 7, 14, 32, 32, XRAM_BANK + S4_TRAIN, S4_TRAIN_FRAME_BYTES, 1},
};

static uint8_t e_type[EN_N], e_color[EN_N], e_hp[EN_N], e_state[EN_N], e_flash[EN_N], e_param[EN_N];
static uint16_t e_timer[EN_N];
static int16_t e_x[EN_N], e_y[EN_N], e_vx[EN_N], e_vy[EN_N], e_base[EN_N];
static uint8_t e_aim[EN_N];
/* Steering round towers: sideways speed (Q4), the x it is making for (0
 * for none), and how wide it weaves, in sixteenths of its full weave. */
static int8_t e_sv[EN_N];
static uint8_t e_tx[EN_N], e_amp[EN_N];

static const uint8_t color_pal[4] = {PAL_ENEMY_RED, PAL_ENEMY_BLUE, PAL_ENEMY_YELLOW, PAL_ENEMY_GRAY};

static mode5_csprite_t *sprite_of(uint8_t i)
{
    return EN_GROUND(e_type[i]) ? &gspr[i] : &spr[SPR_ENEMY + i];
}

void enemies_reset(void)
{
    for (uint8_t i = 0; i < EN_N; i++) {
        e_type[i] = ET_NONE;
        spr_hide(&spr[SPR_ENEMY + i]);
    }
    for (uint8_t i = 0; i < GSPR_MAX; i++)
        spr_hide(&gspr[i]);
}

uint8_t enemies_alive(void)
{
    uint8_t n = 0;
    for (uint8_t i = 0; i < EN_N; i++)
        if (e_type[i] != ET_NONE)
            n++;
    return n;
}

int8_t enemy_spawn(uint8_t type, uint8_t color, int16_t gx, int16_t gy, uint8_t param)
{
    /* Ground units need a plane 0 sprite, which only the low slots have. */
    bool ground = EN_GROUND(type);
    uint8_t last = ground ? gspr_n : EN_N;
    for (uint8_t i = 0; i < last; i++) {
        uint8_t j = ground ? i : (uint8_t)(EN_N - 1 - i); /* air fills from the top */
        if (e_type[j] != ET_NONE)
            continue;
        const etype_t *t = &etypes[type];
        e_type[j] = type;
        e_color[j] = color;
        e_hp[j] = t->hp;
        e_state[j] = 0;
        e_timer[j] = 0;
        e_flash[j] = 0;
        e_param[j] = param;
        e_x[j] = Q4(gx);
        e_y[j] = Q4(gy);
        e_base[j] = Q4(gx);
        e_vx[j] = 0;
        e_vy[j] = 0;
        e_aim[j] = 0;
        e_sv[j] = 0;
        e_tx[j] = 0;
        e_amp[j] = 16;
        spr_setup(sprite_of(j), t->img, PAL_ADDR(color_pal[color]), WH(t->w, t->h), OPT4);
        return (int8_t)j;
    }
    return -1;
}

static void kill(uint8_t i)
{
    spr_hide(sprite_of(i));
    e_type[i] = ET_NONE;
}

static void destroy(uint8_t i)
{
    int16_t gx = e_x[i] >> 4, gy = e_y[i] >> 4;
    const etype_t *t = &etypes[e_type[i]];
    if (e_type[i] == ET_GUNSHIP || e_type[i] == ET_GUNBOAT || e_type[i] == ET_TRAIN) {
        fx_explode(gx, gy, 1);
        fx_explode(gx - 10, gy - 8, 0);
        fx_explode(gx + 10, gy + 6, 0);
        sfx_enemy(SFX_BIG_EXPLODE, SFX_PRI_DESTROY);
    } else {
        fx_explode(gx, gy, e_type[i] == ET_DRONE);
        sfx_enemy(SFX_EXPLODE, SFX_PRI_DESTROY);
    }
    score_add(t->score);
    weapon_xp((uint8_t)(1 + (t->hp >> 2)));
    uint16_t bonus = chain_kill(e_color[i]);
    if (bonus)
        stage_chain_popup(bonus);
    kill(i);
}

/*
 * Flying enemies steer around towers. The waves are laid out clear of
 * them (tools/art/airpath.py), so this is for what that cannot plan: a
 * drone or gunship holding station while the sky slides under it, a dart
 * once it has turned on the player. Every other frame each one looks at
 * the sky rows it covers and the ones it will meet next, and if a tower
 * is in its way picks the nearest place across that is clear; then it
 * banks toward it, gathering sideways speed and easing off as it arrives,
 * and weaves narrower meanwhile. Clear again, it straightens out.
 */
#define STEER_MAX 28 /* sideways speed, Q4: 1.75 px per frame */
#define STEER_ACC 3  /* and how fast that changes */
#define AMP_MIN 5

static bool weaving(uint8_t i)
{
    uint8_t t = e_type[i];
    return t == ET_SAUCER || t == ET_WASP || (t == ET_GUNSHIP && e_state[i] == 1);
}

/* The weave's half width in pixels, narrowed as it steers. */
static uint8_t weave_px(uint8_t i)
{
    uint8_t full = e_type[i] == ET_GUNSHIP ? 40 : e_param[i];
    return e_amp[i] == 16 ? full : (uint8_t)((full * e_amp[i]) >> 4);
}

/* Columns (16 pixels each) that x - hw .. x + hw covers. */
static uint16_t columns(int16_t x, uint8_t hw)
{
    int16_t a = x - hw, b = x + hw;
    if (a < 0)
        a = 0;
    if (b > PF_W - 1)
        b = PF_W - 1;
    return (uint16_t)((2u << (b >> 4)) - (1u << (a >> 4)));
}

static void plan_steer(uint8_t i, int8_t sky_vy)
{
    int16_t gy = e_y[i] >> 4;
    if (gy < -8)
        return;
    const etype_t *t = &etypes[e_type[i]];
    bool weaves = weaving(i);
    /* Moving down the sky faster than it scrolls, it meets the towers below
     * it; slower, those above it come down on it. */
    int8_t ahead = (int8_t)(t->hh + 28);
    if (e_vy[i] <= sky_vy)
        ahead = (int8_t)-ahead;
    uint16_t clear = bg_sky_clear(gy - t->hh) & bg_sky_clear(gy + t->hh) & bg_sky_clear(gy + ahead / 2) &
                     bg_sky_clear(gy + ahead);
    int16_t cx = (weaves ? e_base[i] : e_x[i]) >> 4;
    uint8_t hw = (uint8_t)(t->hw + 2 + (weaves ? weave_px(i) : 0));
    uint16_t need = columns(cx, hw);
    if ((clear & need) == need) {
        e_tx[i] = 0;
        /* Weave wide again only where the whole weave is clear. */
        if (e_amp[i] < 16) {
            uint8_t full = (uint8_t)(t->hw + (e_type[i] == ET_GUNSHIP ? 40 : e_param[i]));
            need = columns(cx, (uint8_t)(full + 2));
            if ((clear & need) == need && cx - full >= 0 && cx + full < PF_W)
                e_amp[i]++;
        }
        return;
    }
    if (weaves && e_amp[i] > AMP_MIN) {
        e_amp[i]--;
        hw = (uint8_t)(t->hw + 2 + weave_px(i));
    }
    /* The nearest clear place, 8 pixels at a time either way. */
    for (int16_t off = 8; off < PF_W; off += 8) {
        for (int8_t side = -1; side <= 1; side += 2) {
            /* Keep turning the way it is already going. */
            int16_t x = cx + (e_sv[i] < 0 ? -side : side) * off;
            if (x - hw < 0 || x + hw >= PF_W)
                continue;
            need = columns(x, hw);
            if ((clear & need) == need) {
                e_tx[i] = (uint8_t)x;
                return;
            }
        }
    }
}

/* Bank toward the target, or straighten out with none. */
static void steer(uint8_t i)
{
    int16_t want = 0;
    if (e_tx[i]) {
        int16_t err = Q4(e_tx[i]) - (weaving(i) ? e_base[i] : e_x[i]);
        if (err > Q4(2) || err < -Q4(2))
            want = err >> 3;
        if (want > STEER_MAX)
            want = STEER_MAX;
        if (want < -STEER_MAX)
            want = -STEER_MAX;
    }
    int16_t sv = e_sv[i];
    if (sv < want)
        sv = sv + STEER_ACC < want ? sv + STEER_ACC : want;
    else if (sv > want)
        sv = sv - STEER_ACC > want ? sv - STEER_ACC : want;
    e_sv[i] = (int8_t)sv;
    e_x[i] += sv;
    e_base[i] += sv;
}

void enemy_damage(uint8_t i, uint8_t dmg)
{
    if (e_type[i] == ET_NONE)
        return;
    e_flash[i] = 3;
    if (e_hp[i] <= dmg)
        destroy(i);
    else
        e_hp[i] -= dmg;
}

void enemies_kill_all(void)
{
    for (uint8_t i = 0; i < EN_N; i++)
        if (e_type[i] != ET_NONE && e_y[i] > 0)
            destroy(i);
}

/* Velocity in Q4 for an angle and a speed in 1/16 px per frame. */
static void set_vel(uint8_t i, uint8_t angle, uint8_t speed)
{
    e_vx[i] = (int16_t)(((int16_t)COS8(angle) * speed) >> 7);
    e_vy[i] = (int16_t)(((int16_t)SIN8(angle) * speed) >> 7);
}

static uint8_t aim_at_player(uint8_t i)
{
    return angle_to((pl_x - e_x[i]) >> 4, (pl_y - e_y[i]) >> 4);
}

static bool on_screen(uint8_t i)
{
    return e_y[i] > Q4(8) && e_y[i] < Q4(PF_H - 40);
}

static void think(uint8_t i)
{
    uint16_t t = e_timer[i]++;
    int16_t gx = e_x[i] >> 4, gy = e_y[i] >> 4;
    bool alive = pl_state == PL_ALIVE;

    switch (e_type[i]) {
    case ET_DART:
        if (e_state[i] == 0) {
            e_vy[i] = 40;
            if (e_y[i] >= Q4(e_param[i])) {
                uint8_t a = aim_at_player(i);
                set_vel(i, a, 44);
                if (alive)
                    bullet_fire(gx, gy + 6, a, 36, BK_PINK);
                e_state[i] = 1;
            }
        }
        break;
    case ET_SAUCER:
        e_vy[i] = 14;
        e_x[i] = e_base[i] + (int16_t)(((int16_t)COS8((uint8_t)(t * 3)) * weave_px(i)) >> 3);
        if ((uint8_t)(t + i * 8) % 72 == 40 && on_screen(i) && alive)
            bullet_fan(gx, gy + 4, aim_at_player(i), 30, BK_PINK, 3, 14);
        break;
    case ET_DRONE:
        if (e_state[i] == 0) {
            e_vy[i] = 28;
            if (e_y[i] >= Q4(e_param[i])) {
                e_state[i] = 1;
                e_timer[i] = 0;
            }
        } else if (e_state[i] == 1) {
            e_vy[i] = 0;
            if (t == 20 || t == 70) {
                uint8_t base = (uint8_t)(t == 20 ? 0 : 16);
                for (uint8_t k = 0; k < 8; k++)
                    bullet_fire(gx, gy, (uint8_t)(base + k * 32), 26, (t == 20) ? BK_BLUE : BK_PINK);
                sfx_enemy(SFX_ENEMY_FIRE, SFX_PRI_FIRE);
            }
            if (t > 120) {
                e_state[i] = 2;
            }
        } else {
            e_vy[i] = -24;
        }
        break;
    case ET_GUNSHIP:
        if (e_state[i] == 0) {
            e_vy[i] = 12;
            if (e_y[i] >= Q4(e_param[i])) {
                e_state[i] = 1;
                e_timer[i] = 0;
            }
        } else if (e_state[i] == 1) {
            e_vy[i] = 0;
            e_x[i] = e_base[i] + (int16_t)(((int16_t)SIN8((uint8_t)t) * weave_px(i)) >> 3);
            if (alive && (t % 48) == 24) {
                for (int8_t k = -1; k <= 1; k++)
                    bullet_fire(gx + k * 8, gy + 14, ANG_DOWN, 40, BK_NEEDLE);
            }
            if (alive && (t % 96) == 60)
                bullet_fan(gx, gy + 8, aim_at_player(i), 24, BK_BLUE_BIG, 3, 20);
            if (t > 560)
                e_state[i] = 2;
        } else {
            e_vy[i] = -16;
        }
        break;
    case ET_TURRET: {
        uint8_t a = aim_at_player(i);
        e_aim[i] = (uint8_t)(((uint8_t)(a - ANG_DOWN + 16)) >> 5);
        if ((uint8_t)(t + i * 13) % 80 == 0 && on_screen(i) && alive) {
            bullet_fire(gx + (COS8(a) >> 4), gy + (SIN8(a) >> 4), a, 28, BK_BLUE);
            sfx_enemy(SFX_ENEMY_FIRE, SFX_PRI_FIRE);
        }
        break;
    }
    case ET_TANK: {
        /* Drives along its road; the turret tracks the player. */
        uint8_t sp = e_param[i] & 0x7F;
        e_vx[i] = (e_param[i] & 0x80) ? -(int16_t)sp : (int16_t)sp;
        uint8_t a = aim_at_player(i);
        e_aim[i] = (uint8_t)(((uint8_t)(a - ANG_DOWN + 16)) >> 5);
        if ((uint8_t)(t + i * 11) % 90 == 30 && on_screen(i) && alive) {
            bullet_fire(gx + (COS8(a) >> 4), gy + (SIN8(a) >> 4), a, 26, BK_PINK);
            sfx_enemy(SFX_ENEMY_FIRE, SFX_PRI_FIRE);
        }
        break;
    }
    case ET_WASP:
        /* Weaves down; stings once on the way. */
        e_vy[i] = 26;
        e_x[i] = e_base[i] + (int16_t)(((int16_t)SIN8((uint8_t)(t * 4)) * weave_px(i)) >> 3);
        if (t == 40 && alive) {
            uint8_t a = aim_at_player(i);
            for (int8_t k = -1; k <= 1; k++)
                bullet_fire(gx, gy + 6, (uint8_t)(a + k * 10), 38, BK_NEEDLE);
        }
        break;
    case ET_TRAIN:
        /* Rolls up the track; gun cars shoot, the locomotive sprays pink. */
        e_vy[i] = -6;
        if (alive && on_screen(i)) {
            uint8_t c = (uint8_t)((t + i * 19) % 96);
            if (e_param[i] && c == 0)
                bullet_fire(gx, gy - 6, aim_at_player(i), 30, BK_BLUE);
            else if (!e_param[i] && c == 48)
                bullet_fan(gx, gy + 12, ANG_DOWN, 24, BK_PINK, 5, 14);
        }
        break;
    case ET_GUNBOAT:
        /* Sails up against the scroll; aimed fans and a pink ring. */
        e_vy[i] = -5;
        if (alive && on_screen(i)) {
            uint8_t c = (uint8_t)((t + i * 23) % 120);
            if (c == 0)
                bullet_fan(gx, gy - 8, aim_at_player(i), 26, BK_BLUE, 3, 14);
            else if (c == 60) {
                for (uint8_t k = 0; k < 10; k++)
                    bullet_fire(gx, gy, (uint8_t)(k * 26 + (t & 31)), 22, BK_PINK);
                sfx_enemy(SFX_ENEMY_FIRE, SFX_PRI_FIRE);
            }
        }
        break;
    }
}

void enemies_update(uint8_t scroll)
{
    uint8_t anim = (uint8_t)(frame_count >> 3);
    /* How fast the sky slides down the screen, Q4. */
    int8_t sky_vy = (int8_t)(((uint16_t)stage->speed * stage->sky_speed) >> 6);
    for (uint8_t i = 0; i < EN_N; i++) {
        uint8_t type = e_type[i];
        if (type == ET_NONE)
            continue;
        think(i);
        if (EN_GROUND(type))
            e_y[i] += (int16_t)scroll << 4;
        else if (bg_hazard) {
            if ((i ^ frame_count) & 1)
                plan_steer(i, sky_vy);
            steer(i);
        }
        e_x[i] += e_vx[i];
        e_y[i] += e_vy[i];
        if (e_y[i] > Q4(PF_H + 40) || e_x[i] < Q4(-40) || e_x[i] > Q4(PF_W + 40) ||
            (e_y[i] < Q4(-48) && e_vy[i] < 0)) {
            kill(i);
            continue;
        }

        const etype_t *t = &etypes[type];
        int16_t gx = e_x[i] >> 4, gy = e_y[i] >> 4;
        tgt_add(gx, gy, t->hw, t->hh, i);
        mode5_csprite_t *s = sprite_of(i);
        uint8_t f;
        if (type == ET_TURRET)
            f = e_aim[i];
        else if (type == ET_TANK) {
            /* Facing left is the right-facing image flipped, so the aim flips too. */
            bool left = e_param[i] & 0x80;
            f = left ? (uint8_t)((8 - e_aim[i]) & 7) : e_aim[i];
            s->options = OPT4 | (left ? MODE5_HFLIP : 0);
        } else if (type == ET_TRAIN)
            f = e_param[i] ? 1 : 0;
        else
            f = (uint8_t)(anim % t->frames);
        s->xram_sprite_ptr = t->img + f * t->frame_bytes;
        if (e_flash[i]) {
            e_flash[i]--;
            s->palette_ptr = PAL_ADDR(PAL_FLASH);
        } else {
            s->palette_ptr = PAL_ADDR(color_pal[e_color[i]]);
        }
        s->x_pos_px = PF_X + gx - (t->w >> 1);
        s->y_pos_px = gy - (t->h >> 1);
    }
}
