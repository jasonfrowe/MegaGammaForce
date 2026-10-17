#ifndef ENEMIES_H
#define ENEMIES_H

#include <stdint.h>

enum
{
    ET_NONE,
    ET_DART,    /* dives at the player */
    ET_SAUCER,  /* weaves down, fires fans */
    ET_DRONE,   /* stops, fires rings, leaves */
    ET_GUNSHIP, /* big, strafes, needles */
    ET_TURRET,  /* ground emplacement, scrolls with the map */
};

#define EN_N 16

void enemies_reset(void);
/* Position in game pixels. Returns the slot, or -1 if full. */
int8_t enemy_spawn(uint8_t type, uint8_t color, int16_t gx, int16_t gy, uint8_t param);
void enemies_update(uint8_t scroll);
void enemy_damage(uint8_t slot, uint8_t dmg);
uint8_t enemies_alive(void);
/* Destroy every enemy on screen, as the hyper sword does to small fry. */
void enemies_kill_all(void);

#endif
