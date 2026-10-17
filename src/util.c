#include "util.h"

static uint8_t rng_a = 0x5A, rng_b = 0xC3;

uint8_t rng8(void)
{
    /* xorshift-ish 16-bit state, cheap on the 6502. */
    uint8_t t = (uint8_t)(rng_a ^ (rng_a << 3));
    rng_a = rng_b;
    rng_b = (uint8_t)(rng_b ^ (rng_b >> 5) ^ t ^ (t >> 2));
    return rng_b;
}

void rng_seed(uint8_t s)
{
    rng_a = (uint8_t)(s | 1);
    rng_b = (uint8_t)(s ^ 0xC3);
}

uint8_t angle_to(int16_t dx, int16_t dy)
{
    uint16_t ux = (uint16_t)(dx < 0 ? -dx : dx);
    uint16_t uy = (uint16_t)(dy < 0 ? -dy : dy);
    while (ux >= 16 || uy >= 16) {
        ux >>= 1;
        uy >>= 1;
    }
    uint8_t a = atan16[(uint8_t)(uy * 16 + ux)];
    if (dx < 0)
        a = (uint8_t)(128 - a);
    if (dy < 0)
        a = (uint8_t)(-a);
    return a;
}
