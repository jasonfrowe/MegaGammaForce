#include "boss.h"
#include "enemies.h"
#include "game.h"
#include "targets.h"
#include "util.h"

uint8_t tgt_n;
uint8_t tgt_x0[TGT_N], tgt_x1[TGT_N], tgt_y0[TGT_N], tgt_y1[TGT_N];
uint8_t tgt_ref[TGT_N];

void tgt_clear(void)
{
    tgt_n = 0;
}

void tgt_add(int16_t gx, int16_t gy, uint8_t hw, uint8_t hh, uint8_t ref)
{
    if (tgt_n >= TGT_N)
        return;
    int16_t x0 = gx + BS_X - hw, x1 = gx + BS_X + hw;
    int16_t y0 = gy + BS_Y - hh, y1 = gy + BS_Y + hh;
    /* Only boxes that reach the visible playfield can be hit. */
    if (x1 < BS_X || x0 >= BS_X + PF_W || y1 < BS_Y || y0 >= BS_Y + PF_H)
        return;
    if (x0 < 0)
        x0 = 0;
    if (y0 < 0)
        y0 = 0;
    if (x1 > 255)
        x1 = 255;
    if (y1 > 255)
        y1 = 255;
    uint8_t i = tgt_n++;
    tgt_x0[i] = (uint8_t)x0;
    tgt_x1[i] = (uint8_t)x1;
    tgt_y0[i] = (uint8_t)y0;
    tgt_y1[i] = (uint8_t)y1;
    tgt_ref[i] = ref;
}

int8_t tgt_hit(uint8_t bx, uint8_t by)
{
    for (uint8_t i = 0; i < tgt_n; i++) {
        if (bx >= tgt_x0[i] && bx < tgt_x1[i] && by >= tgt_y0[i] && by < tgt_y1[i])
            return (int8_t)i;
    }
    return -1;
}

int8_t tgt_nearest(uint8_t bx, uint8_t by)
{
    int8_t best = -1;
    uint16_t best_d = 0xFFFF;
    for (uint8_t i = 0; i < tgt_n; i++) {
        uint8_t cx = (uint8_t)((tgt_x0[i] >> 1) + (tgt_x1[i] >> 1));
        uint8_t cy = (uint8_t)((tgt_y0[i] >> 1) + (tgt_y1[i] >> 1));
        uint16_t d = (uint16_t)absdiff8(cx, bx) + absdiff8(cy, by);
        if (d < best_d) {
            best_d = d;
            best = (int8_t)i;
        }
    }
    return best;
}

void tgt_damage(uint8_t i, uint8_t dmg)
{
    uint8_t ref = tgt_ref[i];
    if (ref >= TGT_BOSS)
        boss_damage((uint8_t)(ref - TGT_BOSS), dmg);
    else
        enemy_damage(ref, dmg);
}
