#ifndef GAME_H
#define GAME_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Coordinates
 *
 * Game space: the playfield, 192 x 240 pixels, origin top left. The
 * playfield is drawn at screen x PF_X, between the two HUD panels.
 *
 * Bullet space: game space shifted so the visible playfield and a margin
 * fit in a byte: bx = gx + BS_X, by = gy + BS_Y. Bullets and shots keep
 * 8.8 fixed point positions in bullet space, so the high byte is the
 * pixel, and leaving the byte range means leaving the screen.
 *
 * Players and enemies use 12.4 fixed point in game space (Q4).
 */
#define PF_X 64
#define PF_W 192
#define PF_H 240
#define BS_X 32
#define BS_Y 8

#define Q4(px) ((int16_t)((px) * 16))

/* Sprites parked here are off the canvas. */
/* Below the tallest sprite (a 64 px image doubled to 128) plus margin. */
#define SPR_HIDDEN_Y (-256)

/* Chain colors. Gray enemies never touch the chain. */
enum
{
    COL_RED,
    COL_BLUE,
    COL_YELLOW,
    COL_GRAY,
};

extern uint8_t frame_count; /* increments every game frame */

#endif
