#include <rp6502.h>
#include "bg.h"
#include "game.h"
#include "xram.h"

/*
 * The whole stage map is in XRAM, top row first, and the 6502 only moves
 * its y position. The map's top 2 * STAGE1_LOOP_ROWS rows are two copies
 * of the same sea, so once the view reaches the top it jumps back one copy
 * and scrolls forever without a seam.
 */

#define MAP_H_PX (STAGE1_MAP_ROWS * 16)
#define LOOP_H_PX (STAGE1_LOOP_ROWS * 16)

uint16_t bg_distance;
bool bg_looping;

static int16_t view_top;  /* map pixel row at the top of the screen */
static uint8_t frac;
static uint8_t speed = 128;
static uint8_t cycle_timer;
static uint8_t cycle_phase;
static uint16_t sea[3]; /* tile palette entries 2-4 */
static uint16_t lamp_red, lamp_green;
static bool pal_dirty;

void bg_init(void)
{
    mode2_config_t cfg = {
        .x_wrap = false,
        .y_wrap = false,
        .x_pos_px = PF_X,
        .y_pos_px = 0,
        .width_tiles = STAGE1_MAP_COLS,
        .height_tiles = STAGE1_MAP_ROWS,
        .xram_data_ptr = XRAM_STAGE_MAP,
        .xram_palette_ptr = PAL_ADDR(PAL_TILES),
        .xram_tile_ptr = XRAM_IMG_TILES,
    };
    xram0_write(XRAM_BG_CONFIG, &cfg, sizeof(cfg));
    xreg_vga_mode2(MODE2_4BPP | MODE2_16X16, XRAM_BG_CONFIG, 0, 0, 0);
    xram0_read(sea, PAL_ADDR(PAL_TILES) + 2, sizeof(sea));
    lamp_red = xram0_peek16(PAL_ADDR(PAL_TILES) + 13 * 2);
    lamp_green = xram0_peek16(PAL_ADDR(PAL_TILES) + 14 * 2);
    bg_reset();
}

void bg_reset(void)
{
    view_top = MAP_H_PX - PF_H;
    frac = 0;
    bg_distance = 0;
    bg_looping = false;
    speed = 128;
}

void bg_set_speed(uint8_t speed_q8)
{
    speed = speed_q8;
}

uint8_t bg_update(void)
{
    uint16_t f = (uint16_t)frac + speed;
    uint8_t px = (uint8_t)(f >> 8);
    frac = (uint8_t)f;
    view_top -= px;
    bg_distance += px;
    if (view_top <= 0) {
        view_top += LOOP_H_PX;
        bg_looping = true;
    }

    /* Palette cycling: the sea shimmers, lamps blink. */
    if (++cycle_timer >= 12) {
        cycle_timer = 0;
        cycle_phase = (uint8_t)((cycle_phase + 1) & 3);
        pal_dirty = true;
    }
    return px;
}

void bg_commit(void)
{
    xram0_poke16(XRAM_BG_CONFIG + offsetof(mode2_config_t, y_pos_px), (uint16_t)(-view_top));
    if (pal_dirty) {
        pal_dirty = false;
        uint16_t base = PAL_ADDR(PAL_TILES);
        /* Rotate sea shades 2-4; shade 1 (deep) stays put. */
        static const uint8_t rot[4][3] = {{0, 1, 2}, {1, 2, 0}, {2, 0, 1}, {1, 0, 2}};
        RIA.addr0 = base + 2 * 2;
        RIA.step0 = 1;
        for (uint8_t i = 0; i < 3; i++) {
            uint16_t c = sea[rot[cycle_phase][i]];
            RIA.rw0 = (uint8_t)c;
            RIA.rw0 = (uint8_t)(c >> 8);
        }
        xram0_poke16(base + 13 * 2, (cycle_phase & 2) ? lamp_red : (uint16_t)(COLOR_FROM_RGB8(80, 10, 20) | COLOR_ALPHA_MASK));
        xram0_poke16(base + 14 * 2, (cycle_phase & 1) ? lamp_green : (uint16_t)(COLOR_FROM_RGB8(30, 90, 70) | COLOR_ALPHA_MASK));
    }
}
