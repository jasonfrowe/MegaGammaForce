#ifndef TARGETS_H
#define TARGETS_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Everything the player can shoot, rebuilt every frame as boxes in
 * bullet space so a hit test is four byte compares. ref says what the
 * box belongs to: an enemy slot, or TGT_BOSS + a boss part.
 */
#define TGT_N 48
#define TGT_BOSS 0x40

extern uint8_t tgt_n;
extern uint8_t tgt_x0[TGT_N], tgt_x1[TGT_N], tgt_y0[TGT_N], tgt_y1[TGT_N];
extern uint8_t tgt_ref[TGT_N];

void tgt_clear(void);
/* Box centered at (gx, gy) game pixels with half sizes hw, hh. Skipped if off screen. */
void tgt_add(int16_t gx, int16_t gy, uint8_t hw, uint8_t hh, uint8_t ref);
/* Index of the first box containing the bullet-space point, or -1. */
int8_t tgt_hit(uint8_t bx, uint8_t by);
/* Index of the box nearest the point, or -1 if there are none. */
int8_t tgt_nearest(uint8_t bx, uint8_t by);
/* Apply damage to whatever box i belongs to. */
void tgt_damage(uint8_t i, uint8_t dmg);

#endif
