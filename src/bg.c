#include <rp6502.h>
#include "bg.h"
#include "game.h"
#include "stage.h"
#include "xram.h"

/*
 * Plane 0's fill is the ground: the stage's whole map is in XRAM, top row
 * first, and the 6502 only moves its y position. The map's top
 * 2 * loop_rows rows are two copies of the same stretch, so once the view
 * reaches the top it jumps back one copy and scrolls forever without a seam.
 *
 * Plane 1's fill is a second tile layer between the ground and everything
 * airborne. A stage can use it as a parallax sky (clouds, and tall
 * structures the ship crashes into), which scrolls faster than the ground,
 * or a boss can take it over to draw itself out of tiles.
 */

#define TILE 16

uint16_t bg_distance;
bool bg_looping;

static int16_t view_top;  /* ground map pixel row at the top of the screen */
static uint16_t map_h_px, loop_h_px;
static uint8_t frac;
static uint8_t speed = 128;

static bool sky_on;
static int16_t sky_top; /* sky map pixel row at the top of the screen */
static uint16_t sky_frac;
static uint16_t sky_map;
static const uint8_t *sky_solid;

static bool layer_dirty;

static uint8_t cycle_timer;
static uint8_t cycle_phase;
static bool pal_dirty;

/* The colors that cycle, read from each stage palette once it is loaded. */
#define CYCLED 3 /* TILES, SKY, LAYER */
static uint16_t shade[CYCLED][3]; /* entries 2-4 */
static uint16_t lamp[CYCLED][2];  /* entries 13-14 */

#define LAMP_RED_DIM ((uint16_t)(COLOR_FROM_RGB8(80, 10, 20) | COLOR_ALPHA_MASK))
#define LAMP_GREEN_DIM ((uint16_t)(COLOR_FROM_RGB8(30, 90, 70) | COLOR_ALPHA_MASK))

/* Far below the canvas, where a layer draws nothing. */
#define LAYER_OFF_Y 1024

static void layer_config(uint16_t map, uint8_t w, uint16_t h, uint16_t tiles, uint8_t pal)
{
    mode2_config_t cfg = {
        .x_wrap = false,
        .y_wrap = false,
        .x_pos_px = PF_X,
        .y_pos_px = LAYER_OFF_Y,
        .width_tiles = w,
        .height_tiles = (int16_t)h,
        .xram_data_ptr = map,
        .xram_palette_ptr = PAL_ADDR(pal),
        .xram_tile_ptr = tiles,
    };
    xram0_write(XRAM_LAYER_CONFIG, &cfg, sizeof(cfg));
}

void bg_init(void)
{
    mode2_config_t cfg = {
        .x_wrap = false,
        .y_wrap = false,
        .x_pos_px = PF_X,
        .y_pos_px = LAYER_OFF_Y,
        .width_tiles = 12,
        .height_tiles = 1,
        .xram_data_ptr = XRAM_BANK,
        .xram_palette_ptr = PAL_ADDR(PAL_TILES),
        .xram_tile_ptr = XRAM_BANK,
    };
    xram0_write(XRAM_BG_CONFIG, &cfg, sizeof(cfg));
    xram0_write(XRAM_LAYER_CONFIG, &cfg, sizeof(cfg));
    xreg_vga_mode2(MODE2_4BPP | MODE2_16X16, XRAM_BG_CONFIG, 0, 0, 0);
    xreg_vga_mode2(MODE2_4BPP | MODE2_16X16, XRAM_LAYER_CONFIG, 1, 0, 0);
}

void bg_load(void)
{
    const stage_def_t *s = stage;
    mode2_config_t cfg = {
        .x_wrap = false,
        .y_wrap = false,
        .x_pos_px = PF_X,
        .y_pos_px = 0,
        .width_tiles = 12,
        .height_tiles = (int16_t)s->map_rows,
        .xram_data_ptr = XRAM_BANK + s->map,
        .xram_palette_ptr = PAL_ADDR(PAL_TILES),
        .xram_tile_ptr = XRAM_BANK + s->tiles,
    };
    xram0_write(XRAM_BG_CONFIG, &cfg, sizeof(cfg));
    map_h_px = s->map_rows * TILE;
    loop_h_px = s->loop_rows * TILE;

    sky_on = s->sky_rows != 0;
    if (sky_on) {
        sky_map = XRAM_BANK + s->sky_map;
        sky_solid = s->sky_solid;
        layer_config(sky_map, 12, s->sky_rows, XRAM_BANK + s->sky_tiles, PAL_SKY);
    } else {
        bg_layer_hide();
    }
    layer_dirty = true;

    for (uint8_t p = 0; p < CYCLED; p++) {
        uint16_t base = PAL_ADDR(PAL_TILES + p);
        xram0_read(shade[p], base + 2 * 2, sizeof(shade[p]));
        lamp[p][0] = xram0_peek16(base + 13 * 2);
        lamp[p][1] = xram0_peek16(base + 14 * 2);
    }
    bg_reset();
}

void bg_reset(void)
{
    view_top = (int16_t)(map_h_px - PF_H);
    frac = 0;
    bg_distance = 0;
    bg_looping = false;
    speed = 128;
    sky_top = (int16_t)(stage->sky_rows * TILE - PF_H);
    sky_frac = 0;
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
        view_top += (int16_t)loop_h_px;
        bg_looping = true;
    }

    /* The sky is nearer, so it scrolls faster, until its last row. */
    if (sky_on && sky_top > 0) {
        sky_frac += (uint16_t)speed * stage->sky_speed / 4;
        sky_top -= (int16_t)(sky_frac >> 8);
        sky_frac &= 0xFF;
        if (sky_top < 0)
            sky_top = 0;
        layer_dirty = true;
    }

    /* Palette cycling: water shimmers, lights run, lamps blink. */
    if (++cycle_timer >= 12) {
        cycle_timer = 0;
        cycle_phase = (uint8_t)((cycle_phase + 1) & 3);
        pal_dirty = true;
    }
    return px;
}

bool bg_sky_hit(int16_t gx, int16_t gy)
{
    if (!sky_on || gx < 0 || gx >= PF_W || gy < 0 || gy >= PF_H)
        return false;
    uint16_t row = (uint16_t)(sky_top + gy) >> 4;
    uint8_t tile = xram0_peek8(sky_map + row * 12 + ((uint8_t)gx >> 4));
    return (sky_solid[tile >> 3] >> (tile & 7)) & 1;
}

void bg_layer_show(uint16_t map, uint8_t w, uint8_t h, uint16_t tiles)
{
    sky_on = false;
    layer_config(map, w, h, tiles, PAL_LAYER);
    layer_dirty = false;
}

void bg_layer_move(int16_t gx, int16_t gy)
{
    xram0_poke16(XRAM_LAYER_CONFIG + offsetof(mode2_config_t, x_pos_px), (uint16_t)(PF_X + gx));
    xram0_poke16(XRAM_LAYER_CONFIG + offsetof(mode2_config_t, y_pos_px), (uint16_t)gy);
}

void bg_layer_hide(void)
{
    sky_on = false;
    layer_dirty = false;
    xram0_poke16(XRAM_LAYER_CONFIG + offsetof(mode2_config_t, y_pos_px), LAYER_OFF_Y);
}

void bg_commit(void)
{
    xram0_poke16(XRAM_BG_CONFIG + offsetof(mode2_config_t, y_pos_px), (uint16_t)(-view_top));
    if (layer_dirty && sky_on) {
        layer_dirty = false;
        xram0_poke16(XRAM_LAYER_CONFIG + offsetof(mode2_config_t, y_pos_px), (uint16_t)(-sky_top));
    }
    if (pal_dirty) {
        pal_dirty = false;
        /* Rotate shades 2-4 through each other. */
        static const uint8_t rot[4][3] = {{0, 1, 2}, {1, 2, 0}, {2, 0, 1}, {1, 0, 2}};
        for (uint8_t p = 0; p < CYCLED; p++) {
            uint16_t base = PAL_ADDR(PAL_TILES + p);
            RIA.addr0 = base + 2 * 2;
            RIA.step0 = 1;
            for (uint8_t i = 0; i < 3; i++) {
                uint16_t c = shade[p][rot[cycle_phase][i]];
                RIA.rw0 = (uint8_t)c;
                RIA.rw0 = (uint8_t)(c >> 8);
            }
            /* A palette with no lamps leaves them transparent. */
            if (lamp[p][0])
                xram0_poke16(base + 13 * 2, (cycle_phase & 2) ? lamp[p][0] : LAMP_RED_DIM);
            if (lamp[p][1])
                xram0_poke16(base + 14 * 2, (cycle_phase & 1) ? lamp[p][1] : LAMP_GREEN_DIM);
        }
    }
}
