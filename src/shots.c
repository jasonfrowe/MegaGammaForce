#include "fx.h"
#include "game.h"
#include "shots.h"
#include "sprites.h"
#include "targets.h"
#include "util.h"

#define SH_N SPR_SHOT_N

static uint16_t sx[SH_N], sy[SH_N]; /* bullet space 8.8 */
static int16_t svx[SH_N], svy[SH_N];
static uint8_t skind[SH_N], slife[SH_N], sang[SH_N], sspeed[SH_N];
static uint8_t snext;

/*
 * Damage per hit. Tuned against the Vulcan (3 per bolt every 4 frames,
 * 45 per second), the strongest weapon:
 *   Spread  1 x 5 orbs every 10 frames  30/s if all hit, ~18/s typical
 *   Homing  1 x 2 darts every 12 frames 10/s, but it never misses
 *   Back    1 per side shot
 */
static const uint8_t kind_dmg[] = {0, 3, 1, 1, 1, 0};

void shots_reset(void)
{
    for (uint8_t i = 0; i < SH_N; i++) {
        skind[i] = SK_NONE;
        spr_hide(&spr[SPR_SHOT + i]);
    }
}

static int8_t alloc(void)
{
    for (uint8_t n = 0; n < SH_N; n++) {
        uint8_t i = snext;
        snext = (uint8_t)(snext + 1 == SH_N ? 0 : snext + 1);
        if (skind[i] == SK_NONE)
            return (int8_t)i;
    }
    return -1;
}

uint8_t shots_count(uint8_t kind)
{
    uint8_t n = 0;
    for (uint8_t i = 0; i < SH_N; i++)
        if (skind[i] == kind)
            n++;
    return n;
}

static void set_velocity(uint8_t i)
{
    svx[i] = vel88(COS8(sang[i]), sspeed[i]);
    svy[i] = vel88(SIN8(sang[i]), sspeed[i]);
}

void shot_fire(uint8_t kind, uint8_t bx, uint8_t by, uint8_t angle, uint8_t speed)
{
    int8_t j = alloc();
    if (j < 0)
        return;
    uint8_t i = (uint8_t)j;
    skind[i] = kind;
    sx[i] = (uint16_t)bx << 8;
    sy[i] = (uint16_t)by << 8;
    sang[i] = angle;
    sspeed[i] = speed;
    slife[i] = kind == SK_HOMING ? 90 : 255;
    set_velocity(i);
    mode5_csprite_t *s = &spr[SPR_SHOT + i];
    switch (kind) {
    case SK_VULCAN:
        spr_setup(s, XRAM_IMG_VULCAN, PAL_ADDR(PAL_SHOTS), WH(8, 16), OPT4);
        break;
    case SK_SPREAD:
        spr_setup(s, XRAM_IMG_SHOT8 + SHOT8_SPREAD * SHOT8_FRAME_BYTES, PAL_ADDR(PAL_SHOTS), WH(8, 8), OPT4);
        break;
    case SK_HOMING:
        spr_setup(s, XRAM_IMG_SHOT8 + SHOT8_HOMING * SHOT8_FRAME_BYTES, PAL_ADDR(PAL_SHOTS), WH(8, 8), OPT4);
        break;
    case SK_BACK:
        spr_setup(s, XRAM_IMG_SHOT8 + SHOT8_BACK * SHOT8_FRAME_BYTES, PAL_ADDR(PAL_SHOTS), WH(8, 8), OPT4);
        break;
    }
}

void shot_spark(uint8_t bx, uint8_t by)
{
    int8_t j = alloc();
    if (j < 0)
        return;
    uint8_t i = (uint8_t)j;
    skind[i] = SK_SPARK;
    slife[i] = 1;
    sx[i] = (uint16_t)bx << 8;
    sy[i] = (uint16_t)by << 8;
    svx[i] = svy[i] = 0;
    mode5_csprite_t *s = &spr[SPR_SHOT + i];
    spr_setup(s, XRAM_IMG_SHOT8 + (SHOT8_SPARK + (rng8() & 1)) * SHOT8_FRAME_BYTES, PAL_ADDR(PAL_SHOTS), WH(8, 8),
              OPT4 | ((rng8() & 1) ? MODE5_HFLIP : 0));
    s->x_pos_px = (int16_t)bx + (PF_X - BS_X - 4);
    s->y_pos_px = (int16_t)by - (BS_Y + 4);
}

static void kill(uint8_t i)
{
    skind[i] = SK_NONE;
    spr_hide(&spr[SPR_SHOT + i]);
}

void shots_update(void)
{
    mode5_csprite_t *s = &spr[SPR_SHOT];
    for (uint8_t i = 0; i < SH_N; i++, s++) {
        uint8_t k = skind[i];
        if (k == SK_NONE)
            continue;
        if (k == SK_SPARK) {
            if (slife[i]-- == 0)
                kill(i);
            continue;
        }
        if (k == SK_HOMING) {
            if (--slife[i] == 0) {
                kill(i);
                continue;
            }
            /* Steer toward the nearest target every other frame. */
            if ((i ^ frame_count) & 1) {
                int8_t t = tgt_nearest((uint8_t)(sx[i] >> 8), (uint8_t)(sy[i] >> 8));
                if (t >= 0) {
                    int16_t tx = (int16_t)((tgt_x0[(uint8_t)t] + tgt_x1[(uint8_t)t]) >> 1);
                    int16_t ty = (int16_t)((tgt_y0[(uint8_t)t] + tgt_y1[(uint8_t)t]) >> 1);
                    uint8_t want = angle_to(tx - (int16_t)(sx[i] >> 8), ty - (int16_t)(sy[i] >> 8));
                    int8_t diff = (int8_t)(want - sang[i]);
                    if (diff > 14)
                        diff = 14;
                    else if (diff < -14)
                        diff = -14;
                    sang[i] = (uint8_t)(sang[i] + diff);
                    set_velocity(i);
                }
            }
        }
        sx[i] += (uint16_t)svx[i];
        sy[i] += (uint16_t)svy[i];
        uint8_t x = (uint8_t)(sx[i] >> 8);
        uint8_t y = (uint8_t)(sy[i] >> 8);
        if (x < 20 || x > 236 || y < 4 || y > 250) {
            kill(i);
            continue;
        }
        int8_t t = tgt_hit(x, y);
        if (t < 0 && k == SK_VULCAN)
            t = tgt_hit(x, (uint8_t)(y + 8)); /* the bolt is 16 tall */
        if (t >= 0) {
            tgt_damage((uint8_t)t, kind_dmg[k]);
            fx_spark(x, y);
            kill(i);
            continue;
        }
        if (k == SK_VULCAN) {
            s->x_pos_px = (int16_t)x + (PF_X - BS_X - 4);
            s->y_pos_px = (int16_t)y - BS_Y;
        } else {
            s->x_pos_px = (int16_t)x + (PF_X - BS_X - 4);
            s->y_pos_px = (int16_t)y - (BS_Y + 4);
        }
    }
}
