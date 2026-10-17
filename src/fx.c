#include "fx.h"
#include "game.h"
#include "sprites.h"

#define FX_N SPR_FX_N
#define FX_TICKS 4 /* frames per animation frame */

static int16_t fx_x[FX_N], fx_y[FX_N]; /* canvas top left */
static uint8_t fx_kind[FX_N];         /* 0 free, 1 small, 2 big, 3 spark */
static uint8_t fx_frame[FX_N], fx_tick[FX_N];
static uint8_t fx_next;

void fx_reset(void)
{
    for (uint8_t i = 0; i < FX_N; i++) {
        fx_kind[i] = 0;
        spr_hide(&spr[SPR_FX + i]);
    }
}

static uint8_t alloc(void)
{
    for (uint8_t n = 0; n < FX_N; n++) {
        uint8_t i = fx_next;
        fx_next = (uint8_t)(fx_next + 1 == FX_N ? 0 : fx_next + 1);
        if (fx_kind[i] == 0)
            return i;
    }
    /* All busy: recycle the next one. */
    uint8_t i = fx_next;
    fx_next = (uint8_t)(fx_next + 1 == FX_N ? 0 : fx_next + 1);
    return i;
}

static void start(uint8_t i, uint8_t kind, int16_t x, int16_t y)
{
    mode5_csprite_t *s = &spr[SPR_FX + i];
    fx_kind[i] = kind;
    fx_frame[i] = 0;
    fx_tick[i] = 0;
    fx_x[i] = x;
    fx_y[i] = y;
    if (kind == 2)
        spr_setup(s, XRAM_IMG_EXPL32, PAL_ADDR(PAL_EXPLOSION), WH(32, 32), OPT4);
    else
        spr_setup(s, XRAM_IMG_EXPL16, PAL_ADDR(PAL_EXPLOSION), WH(16, 16), OPT4);
    s->x_pos_px = x;
    s->y_pos_px = y;
}

void fx_explode(int16_t gx, int16_t gy, uint8_t big)
{
    uint8_t h = big ? 16 : 8;
    start(alloc(), big ? 2 : 1, PF_X + gx - h, gy - h);
}

void fx_spark(uint8_t bx, uint8_t by)
{
    uint8_t i = alloc();
    start(i, 3, (int16_t)bx + (PF_X - BS_X - 8), (int16_t)by - (BS_Y + 8));
    fx_frame[i] = 0;
}

void fx_update(uint8_t scroll)
{
    mode5_csprite_t *s = &spr[SPR_FX];
    for (uint8_t i = 0; i < FX_N; i++, s++) {
        uint8_t k = fx_kind[i];
        if (k == 0)
            continue;
        if (++fx_tick[i] >= (k == 3 ? 2 : FX_TICKS)) {
            fx_tick[i] = 0;
            uint8_t last = k == 3 ? 2 : (k == 2 ? EXPL32_FRAMES : EXPL16_FRAMES);
            if (++fx_frame[i] >= last) {
                fx_kind[i] = 0;
                spr_hide(s);
                continue;
            }
        }
        fx_y[i] += scroll;
        s->y_pos_px = fx_y[i];
        s->xram_sprite_ptr = (k == 2 ? XRAM_IMG_EXPL32 + fx_frame[i] * EXPL32_FRAME_BYTES
                                     : XRAM_IMG_EXPL16 + fx_frame[i] * EXPL16_FRAME_BYTES);
    }
}
