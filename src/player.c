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
static uint8_t shown_weapon = 0xFF, shown_gauge = 0xFF, shown_lives = 0xFF;

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
    shown_weapon = shown_gauge = shown_lives = 0xFF;
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
    for (uint8_t i = 0; i < tgt_n; i++) {
        if (tgt_x1[i] > sword_x0 && tgt_x0[i] < sword_x1 && tgt_y1[i] > sword_y0 && tgt_y0[i] < sword_y1)
            tgt_damage(i, dmg);
    }
}

static void fire(uint8_t w)
{
    uint8_t nx = pl_bx, ny = (uint8_t)(pl_by - 10);
    if (fire_cd)
        fire_cd--;

    switch (w) {
    case WP_VULCAN:
        if (fire_cd == 0 && shots_count(SK_VULCAN) < 8) {
            shot_fire(SK_VULCAN, nx, (uint8_t)(ny - 4), ANG_UP, 128);
            fire_cd = 4;
            sfx_player(SFX_VULCAN, SFX_PRI_FIRE);
        }
        break;
    case WP_HOMING:
        if (fire_cd == 0 && shots_count(SK_HOMING) < 8) {
            shot_fire(SK_HOMING, (uint8_t)(pl_bx - 10), pl_by, ANG_UP - 40, 60);
            shot_fire(SK_HOMING, (uint8_t)(pl_bx + 10), pl_by, ANG_UP + 40, 60);
            fire_cd = 12;
            sfx_player(SFX_HOMING, SFX_PRI_FIRE);
        }
        break;
    case WP_SPREAD:
        if (fire_cd == 0) {
            for (int8_t a = -24; a <= 24; a += 12)
                shot_fire(SK_SPREAD, nx, ny, (uint8_t)(ANG_UP + a), 88);
            fire_cd = 10;
            sfx_player(SFX_SPREAD, SFX_PRI_FIRE);
        }
        break;
    case WP_BACKWIDE:
        if (fire_cd == 0) {
            shot_fire(SK_VULCAN, nx, (uint8_t)(ny - 4), ANG_UP, 128);
            shot_fire(SK_BACK, (uint8_t)(pl_bx - 6), pl_by, ANG_DOWN + 36, 80);
            shot_fire(SK_BACK, (uint8_t)(pl_bx + 6), pl_by, ANG_DOWN - 36, 80);
            shot_fire(SK_BACK, (uint8_t)(pl_bx - 8), pl_by, 128 + 12, 80);
            shot_fire(SK_BACK, (uint8_t)(pl_bx + 8), pl_by, (uint8_t)(0 - 12), 80);
            fire_cd = 8;
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
            if (fire_cd == 0) {
                tgt_damage((uint8_t)t, 2);
                fire_cd = 3;
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
    sword_active = hyper || w == WP_SWORD || w == WP_HYPER;
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
        if ((sword_tick & 1) == 0)
            sword_strike(4);
        /* The blade's energy also lances up the screen as a beam. */
        int16_t top = sy - 16 - 128;
        for (uint8_t k = 0; k < SPR_BEAM_N; k++) {
            mode5_csprite_t *b = &spr[SPR_BEAM + k];
            spr_setup(b, XRAM_IMG_BEAM + ((sword_tick >> 2) & 1) * BEAM_FRAME_BYTES, PAL_ADDR(PAL_SHOTS), WH(32, 64),
                      OPT4 | MODE5_VDOUBLE);
            b->x_pos_px = sx - 16;
            b->y_pos_px = top - 128 * k;
        }
        if ((sword_tick & 1) == 1) {
            uint8_t x0 = (uint8_t)(pl_bx - 14), x1 = (uint8_t)(pl_bx + 14);
            for (uint8_t i = 0; i < tgt_n; i++)
                if (tgt_x1[i] > x0 && tgt_x0[i] < x1 && tgt_y0[i] < pl_by)
                    tgt_damage(i, 3);
        }
    } else {
        spr_setup(s, XRAM_IMG_SWORD + f * SWORD_FRAME_BYTES, PAL_ADDR(PAL_SHOTS), WH(48, 32), OPT4);
        s->x_pos_px = sx - 24;
        s->y_pos_px = sy - 34;
        sword_x0 = pl_bx > 24 ? (uint8_t)(pl_bx - 24) : 0;
        sword_x1 = pl_bx < 231 ? (uint8_t)(pl_bx + 24) : 255;
        sword_y0 = pl_by > 36 ? (uint8_t)(pl_by - 36) : 0;
        sword_y1 = (uint8_t)(pl_by - 4);
        if ((sword_tick & 3) == 0)
            sword_strike(3);
        if ((sword_tick & 15) == 0)
            sfx_player(SFX_SWORD, SFX_PRI_FIRE);
    }
}

void player_update(bool controls)
{
    if (pl_state == PL_DEAD) {
        if (dead_timer && --dead_timer == 0 && pl_lives)
            player_respawn();
        return;
    }

    uint8_t held = controls ? in_held : 0;
    uint8_t w = (uint8_t)((held >> 4) & 7);

    /* Hyper sword: all three buttons with a full gauge. */
    if (w == WP_HYPER && sword_gauge >= SWORD_GAUGE_MAX && hyper_timer == 0) {
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
    if (invuln)
        invuln--;

    if (w)
        pl_weapon = w;
    if (w == WP_HYPER && hyper_timer == 0)
        w = WP_SWORD; /* not charged: just the sword */
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

static const char *const weapon_name[8][2] = {
    {"", ""},
    {"VULCAN", ""},
    {"HOMING", ""},
    {"BACK", "WIDE"},
    {"SPREAD", ""},
    {"RADIANT", "SWORD"},
    {"HOMING", "PLASMA"},
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
