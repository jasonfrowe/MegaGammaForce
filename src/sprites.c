#include <string.h>
#include "game.h"
#include "sprites.h"

mode5_csprite_t spr[SPR_COUNT];
mode5_csprite_t gspr[GSPR_N];

void spr_hide_all(void)
{
    for (uint8_t i = 0; i < SPR_COUNT; i++) {
        spr[i].x_pos_px = 0;
        spr[i].y_pos_px = SPR_HIDDEN_Y;
        spr[i].xram_sprite_ptr = XRAM_IMG_BULLET8;
        spr[i].palette_ptr = PAL_ADDR(PAL_BULLET_PINK);
        spr[i].width_height = WH(8, 8);
        spr[i].options = OPT4;
    }
    for (uint8_t i = 0; i < GSPR_N; i++) {
        gspr[i] = spr[0];
    }
}

void spr_init(void)
{
    spr_hide_all();
    spr_commit();
    /* Ground targets on plane 0, over the tile map. */
    xreg_vga_mode5(MODE5_CUSTOM, XRAM_GSPR, GSPR_N, 0, 0, 0);
    /* Everything airborne on plane 1. */
    xreg_vga_mode5(MODE5_CUSTOM, XRAM_SPR, SPR_COUNT, 1, 0, 0);
}

void spr_commit(void)
{
    xram0_write(XRAM_SPR, spr, sizeof(spr));
    xram0_write(XRAM_GSPR, gspr, sizeof(gspr));
}
