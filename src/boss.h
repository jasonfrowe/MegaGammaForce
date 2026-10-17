#ifndef BOSS_H
#define BOSS_H

#include <stdbool.h>
#include <stdint.h>

void boss_reset(void);
void boss_start(void);
void boss_update(void);
void boss_damage(uint8_t part, uint8_t dmg);
bool boss_done(void);
void boss_hud(bool force);

extern uint8_t boss_destruction; /* percent of the boss destroyed */
extern bool boss_escaped;

#endif
