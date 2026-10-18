#ifndef ENEMIES_H
#define ENEMIES_H

#include <stdint.h>
#include "xram.h"

enum
{
    ET_NONE,
    ET_DART,    /* dives at the player */
    ET_SAUCER,  /* weaves down, fires fans */
    ET_DRONE,   /* stops, fires rings, leaves */
    ET_GUNSHIP, /* big, strafes, needles */
    ET_TURRET,  /* ground emplacement, scrolls with the map */
    ET_TANK,    /* ground, drives along a road; param: speed, bit 7 = left */
    ET_WASP,    /* weaves down, stings with aimed needles */
    ET_GUNBOAT, /* on the sea, sails up the screen, heavy guns */
    ET_TRAIN,   /* armoured car on stage 4's railway; param 0 = locomotive */
};

/* Slots below gspr_n can hold ground units, on plane 0's sprites. */
#define EN_N SPR_ENEMY_N
#define EN_GROUND(type) ((type) == ET_TURRET || (type) == ET_TANK || (type) == ET_GUNBOAT || (type) == ET_TRAIN)

void enemies_reset(void);
/* Position in game pixels. Returns the slot, or -1 if full. */
int8_t enemy_spawn(uint8_t type, uint8_t color, int16_t gx, int16_t gy, uint8_t param);
void enemies_update(uint8_t scroll);
void enemy_damage(uint8_t slot, uint8_t dmg);
uint8_t enemies_alive(void);
/* Destroy every enemy on screen, as the hyper sword does to small fry. */
void enemies_kill_all(void);

#endif
