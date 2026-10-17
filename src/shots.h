#ifndef SHOTS_H
#define SHOTS_H

#include <stdint.h>

/* Player projectiles. */
enum
{
    SK_NONE,
    SK_VULCAN,
    SK_SPREAD,
    SK_HOMING,
    SK_BACK,
    SK_SPARK, /* homing plasma link, drawn for one frame */
};

void shots_reset(void);
/* From bullet-space pixel (bx, by), angle and speed in 1/16 px per frame. */
void shot_fire(uint8_t kind, uint8_t bx, uint8_t by, uint8_t angle, uint8_t speed);
/* A plasma link segment at a bullet-space point. */
void shot_spark(uint8_t bx, uint8_t by);
uint8_t shots_count(uint8_t kind);
void shots_update(void);

#endif
