#include "bullets.h"
#include "bg.h"
#include "fx.h"
#include "game.h"
#include "player.h"
#include "sprites.h"
#include "util.h"

#define BL_N SPR_BULLET_N

/* Bullet space, 8.8 fixed point; see game.h. */
static uint16_t bx[BL_N], by[BL_N];
static int16_t bvx[BL_N], bvy[BL_N];
static uint8_t bkind[BL_N];
static uint8_t next_free;
uint8_t bullet_count;

static const uint16_t kind_img[] = {
    0,
    XRAM_IMG_BULLET8 + BULLET8_ORB * BULLET8_FRAME_BYTES,
    XRAM_IMG_BULLET8 + BULLET8_ORB * BULLET8_FRAME_BYTES,
    XRAM_IMG_BULLET16,
    XRAM_IMG_BULLET16,
    XRAM_IMG_BULLET8 + BULLET8_NEEDLE * BULLET8_FRAME_BYTES,
};

void bullets_reset(void)
{
    for (uint8_t i = 0; i < BL_N; i++) {
        bkind[i] = BK_NONE;
        spr_hide(&spr[SPR_BULLET + i]);
    }
    bullet_count = 0;
    next_free = 0;
}

void bullet_fire(int16_t gx, int16_t gy, uint8_t angle, uint8_t speed, uint8_t kind)
{
    int16_t x = gx + BS_X, y = gy + BS_Y;
    if (x < 0 || x > 255 || y < 0 || y > 255)
        return;
    uint8_t i = next_free;
    for (uint8_t n = 0; n < BL_N; n++) {
        if (bkind[i] == BK_NONE)
            goto found;
        i = (uint8_t)((i + 1) & (BL_N - 1));
    }
    return;
found:
    next_free = (uint8_t)((i + 1) & (BL_N - 1));
    bullet_count++;
    bkind[i] = kind;
    bx[i] = (uint16_t)x << 8;
    by[i] = (uint16_t)y << 8;
    bvx[i] = vel88(COS8(angle), speed);
    bvy[i] = vel88(SIN8(angle), speed);
    bool big = kind == BK_PINK_BIG || kind == BK_BLUE_BIG;
    spr_setup(&spr[SPR_BULLET + i], kind_img[kind],
              PAL_ADDR(BK_IS_PINK(kind) ? PAL_BULLET_PINK : PAL_BULLET_BLUE),
              big ? WH(16, 16) : WH(8, 8), OPT4);
}

void bullet_fan(int16_t gx, int16_t gy, uint8_t angle, uint8_t speed, uint8_t kind, uint8_t n, uint8_t step)
{
    uint8_t a = (uint8_t)(angle - (uint8_t)((n - 1) * step / 2));
    for (uint8_t i = 0; i < n; i++) {
        bullet_fire(gx, gy, a, speed, kind);
        a += step;
    }
}

static void kill(uint8_t i)
{
    bkind[i] = BK_NONE;
    bullet_count--;
    spr_hide(&spr[SPR_BULLET + i]);
}

void bullets_update(void)
{
    uint8_t px = pl_bx, py = pl_by;
    bool vulnerable = player_vulnerable();
    bool anim = (frame_count & 7) == 0;
    uint8_t anim_frame = (uint8_t)((frame_count >> 3) & 1);
    mode5_csprite_t *s = &spr[SPR_BULLET];

    for (uint8_t i = 0; i < BL_N; i++, s++) {
        uint8_t k = bkind[i];
        if (k == BK_NONE)
            continue;
        bx[i] += (uint16_t)bvx[i];
        by[i] += (uint16_t)bvy[i];
        uint8_t x = (uint8_t)(bx[i] >> 8);
        uint8_t y = (uint8_t)(by[i] >> 8);
        if (x < 20 || x > 236 || y < 2 || y > 250) {
            kill(i);
            continue;
        }
        /* A tower stops a bullet; half of them are checked each frame. */
        if (bg_hazard && ((i ^ frame_count) & 1) && bg_sky_hit_bs(x, y)) {
            kill(i);
            continue;
        }
        bool big = k == BK_PINK_BIG || k == BK_BLUE_BIG;
        uint8_t r = big ? 5 : 3;
        if (sword_active && (BK_IS_PINK(k) || hyper_timer) && x >= sword_x0 && x < sword_x1 && y >= sword_y0 && y < sword_y1) {
            sword_absorb();
            fx_spark(x, y);
            kill(i);
            continue;
        }
        if (vulnerable && absdiff8(x, px) < r && absdiff8(y, py) < r) {
            kill(i);
            player_hit();
            vulnerable = false;
            continue;
        }
        /* Bullet space to canvas: x - BS_X + PF_X - half width. */
        if (big) {
            s->x_pos_px = (int16_t)x + (PF_X - BS_X - 8);
            s->y_pos_px = (int16_t)y - (BS_Y + 8);
        } else {
            s->x_pos_px = (int16_t)x + (PF_X - BS_X - 4);
            s->y_pos_px = (int16_t)y - (BS_Y + 4);
        }
        if (anim && k != BK_NEEDLE)
            s->xram_sprite_ptr = kind_img[k] + anim_frame * (big ? BULLET16_FRAME_BYTES : BULLET8_FRAME_BYTES);
    }
}

uint8_t bullets_cancel_all(void)
{
    uint8_t n = 0;
    for (uint8_t i = 0; i < BL_N; i++) {
        if (bkind[i] != BK_NONE) {
            if ((n & 3) == 0)
                fx_spark((uint8_t)(bx[i] >> 8), (uint8_t)(by[i] >> 8));
            kill(i);
            n++;
        }
    }
    return n;
}
