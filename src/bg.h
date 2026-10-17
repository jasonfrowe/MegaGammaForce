#ifndef BG_H
#define BG_H

#include <stdbool.h>
#include <stdint.h>

void bg_init(void);
void bg_reset(void);
/* Advance the scroll; returns pixels scrolled this frame (0-3). */
uint8_t bg_update(void);
void bg_commit(void);
void bg_set_speed(uint8_t speed_q8); /* pixels per frame in 1/256 */

/* Pixels scrolled since the stage began. Drives the stage script. */
extern uint16_t bg_distance;
/* True once the scroll has reached the looping sea at the top of the map. */
extern bool bg_looping;

#endif
