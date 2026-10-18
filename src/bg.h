#ifndef BG_H
#define BG_H

#include <stdbool.h>
#include <stdint.h>

void bg_init(void);
/* Point the layers at the current stage's bank, once it is loaded. */
void bg_load(void);
void bg_reset(void);
/* Advance the scroll; returns pixels scrolled this frame (0-3). */
uint8_t bg_update(void);
void bg_commit(void);
void bg_set_speed(uint8_t speed_q8); /* pixels per frame in 1/256 */

/*
 * Towers. On a stage whose sky has solid tiles (bg_hazard), the ship
 * crashes into them, most shots and every enemy bullet stop at them, and
 * flying enemies steer around them. Missiles, plasma, the swords and the
 * hyper beam pass through.
 */
extern bool bg_hazard;
/* Is there a solid sky tile at this game pixel? */
bool bg_sky_hit(int16_t gx, int16_t gy);
/* The same, at a bullet-space pixel. */
bool bg_sky_hit_bs(uint8_t bx, uint8_t by);

/* A boss takes over plane 1's layer: a w x h tile map, PAL_LAYER colors.
 * bg_layer_move() places its top left in game pixels. */
void bg_layer_show(uint16_t map, uint8_t w, uint8_t h, uint16_t tiles);
void bg_layer_move(int16_t gx, int16_t gy);
void bg_layer_hide(void);

/* Pixels scrolled since the stage began. Drives the stage script. */
extern uint16_t bg_distance;
/* True once the scroll has reached the looping stretch at the top of the map. */
extern bool bg_looping;

#endif
