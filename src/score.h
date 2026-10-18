#ifndef SCORE_H
#define SCORE_H

#include <stdbool.h>
#include <stdint.h>

#define SCORE_DIGITS 7

extern uint8_t score_d[SCORE_DIGITS]; /* most significant first */
extern uint8_t hiscore_d[SCORE_DIGITS];
#define SCORE_EXTEND_EVERY 100000 /* an extra life at each multiple */
extern uint8_t score_extends; /* extra lives earned, not yet handed out */

/*
 * The chain, after Radiant Silvergun: destroy three enemies of one color
 * in a row to complete a set. Each completed set raises the chain and pays
 * a growing bonus. A different color part way through a set breaks the
 * chain. After a completed set, any color may start the next one.
 */
extern uint8_t chain_level;
extern uint8_t chain_max;
extern uint8_t chain_color;
extern uint8_t chain_count; /* kills in the current color run, 0-3 (3 = set complete) */
extern uint16_t chain_last_bonus;

void score_reset(void);
void score_add(uint16_t pts);
/* Register a kill. Returns the bonus paid, 0 if no set was completed. */
uint16_t chain_kill(uint8_t color);
/* Lose the chain (on death); chain_max is kept for the results. */
void chain_break(void);
void score_hud(bool force);
/* Just the high score, for the title screen. */
void score_hud_hiscore(void);
void hiscore_load(void);
void hiscore_save(void);

#endif
