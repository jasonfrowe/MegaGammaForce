#ifndef BULLETS_H
#define BULLETS_H

#include <stdbool.h>
#include <stdint.h>

/* Enemy bullets. Pink ones can be absorbed by the radiant sword. */
enum
{
    BK_NONE,
    BK_PINK,
    BK_BLUE,
    BK_PINK_BIG,
    BK_BLUE_BIG,
    BK_NEEDLE, /* pink, drawn pointing down */
};

#define BK_IS_PINK(k) ((k) == BK_PINK || (k) == BK_PINK_BIG || (k) == BK_NEEDLE)

extern uint8_t bullet_count;

void bullets_reset(void);
/* Fire from game pixel (gx, gy) at angle with speed in 1/16 px per frame. */
void bullet_fire(int16_t gx, int16_t gy, uint8_t angle, uint8_t speed, uint8_t kind);
/* n bullets centred on angle, step apart. */
void bullet_fan(int16_t gx, int16_t gy, uint8_t angle, uint8_t speed, uint8_t kind, uint8_t n, uint8_t step);
void bullets_update(void);
/* Remove every bullet, converting each to points. Returns how many. */
uint8_t bullets_cancel_all(void);

#endif
