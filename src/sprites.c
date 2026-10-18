#include <string.h>
#include "game.h"
#include "sprites.h"

mode5_csprite_t spr[SPR_COUNT];
mode5_csprite_t gspr[GSPR_MAX];
uint8_t gspr_n = 8;
mode5_csprite_t hspr[HSPR_N];

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
    for (uint8_t i = 0; i < GSPR_MAX; i++)
        gspr[i] = spr[0];
    for (uint8_t i = 0; i < HSPR_N; i++)
        hspr[i] = spr[0];
}

void spr_init(void)
{
    spr_hide_all();
    spr_commit();
    /* Ground units on plane 0, over the tile map. */
    spr_ground_count(gspr_n);
    /* HUD graphics on plane 2, over the text. */
    xreg_vga_mode5(MODE5_CUSTOM, XRAM_HSPR, HSPR_N, 2, 0, 0);
    /* Everything airborne on plane 1. */
    xreg_vga_mode5(MODE5_CUSTOM, XRAM_SPR, SPR_COUNT, 1, 0, 0);
}

void spr_ground_count(uint8_t n)
{
    gspr_n = n;
    xreg_vga_mode5(MODE5_CUSTOM, XRAM_GSPR, n, 0, 0, 0);
}

void spr_commit(void)
{
    xram0_write(XRAM_SPR, spr, sizeof(spr));
    xram0_write(XRAM_GSPR, gspr, sizeof(gspr));
    xram0_write(XRAM_HSPR, hspr, sizeof(hspr));
}
