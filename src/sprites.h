#ifndef SPRITES_H
#define SPRITES_H

#include <stdint.h>
#include "game.h"
#include "xram.h"

/* RAM mirrors of the sprite lists. Game code edits these, and
 * spr_commit() copies them to XRAM once per frame, right after VSYNC. */
extern mode5_csprite_t spr[SPR_COUNT];
extern mode5_csprite_t gspr[GSPR_N];

#define WH(w, h) MODE5_SIZE(w, h)
#define OPT4 MODE5_4BPP

void spr_init(void);
void spr_hide_all(void);
void spr_commit(void);

static inline void spr_hide(mode5_csprite_t *s)
{
    s->y_pos_px = SPR_HIDDEN_Y;
}

static inline void spr_setup(mode5_csprite_t *s, uint16_t img, uint16_t pal, uint8_t wh, uint8_t opt)
{
    s->xram_sprite_ptr = img;
    s->palette_ptr = pal;
    s->width_height = wh;
    s->options = opt;
}

#endif
