#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

extern const int8_t cos8[256];
extern const uint8_t atan16[256];

#define COS8(a) (cos8[(uint8_t)(a)])
#define SIN8(a) (cos8[(uint8_t)((a) - 64)])

/* Angles: 256 to a circle, 0 right, 64 down, 128 left, 192 up. */
#define ANG_DOWN 64
#define ANG_UP 192

uint8_t rng8(void);
void rng_seed(uint8_t s);

/* Angle from the origin to (dx, dy). */
uint8_t angle_to(int16_t dx, int16_t dy);

/* Velocity component in 8.8 pixels per frame for a speed in 1/16 pixels per frame. */
static inline int16_t vel88(int8_t trig, uint8_t speed16)
{
    return (int16_t)(((int16_t)trig * (int16_t)speed16) >> 3);
}

static inline uint8_t absdiff8(uint8_t a, uint8_t b)
{
    return a > b ? (uint8_t)(a - b) : (uint8_t)(b - a);
}

#endif
