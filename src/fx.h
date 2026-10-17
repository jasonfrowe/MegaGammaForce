#ifndef FX_H
#define FX_H

#include <stdint.h>

void fx_reset(void);
/* Explosion centered at game pixel (gx, gy). big: 32x32, else 16x16. */
void fx_explode(int16_t gx, int16_t gy, uint8_t big);
/* Small flash at a bullet-space point (hits, absorbed bullets). */
void fx_spark(uint8_t bx, uint8_t by);
/* Explosions drift down with the scroll when scroll is nonzero. */
void fx_update(uint8_t scroll);

#endif
