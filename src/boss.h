#ifndef BOSS_H
#define BOSS_H

#include <stdbool.h>
#include <stdint.h>
#include "sprites.h"

/*
 * A boss is a set of parts, each with its own hit points. Shooting a part
 * off adds its weight to the destruction percentage; destroying the core
 * ends the fight. Take everything else apart first for 100%.
 *
 * boss.c runs what every boss shares (entry, damage, wrecks, dying,
 * escaping on the time limit, the HUD). A boss_def_t gives the parts and
 * the hooks that make each boss its own.
 */

typedef struct boss_def boss_def_t;

#define BOSS_PARTS_MAX SPR_BOSS_N
#define NO_PARENT 0xFF

enum
{
    PK_SPRITE, /* a sprite, drawn with PAL_BOSS_WRECK once destroyed */
    PK_TURRET, /* a sprite with 8 aim frames, hidden once destroyed */
    PK_HIDDEN, /* no sprite of its own: drawn in the boss's tile layer */
};

typedef struct
{
    int16_t ox, oy;  /* center offset from the boss center */
    uint8_t w, h;    /* sprite size */
    uint8_t hw, hh;  /* hitbox half sizes */
    uint16_t hp;
    uint8_t weight;  /* destruction percent */
    uint16_t img;    /* XRAM address of the image */
    uint8_t opt;     /* extra MODE5 options, such as HFLIP */
    uint8_t kind;
    uint8_t parent;  /* destroyed along with this part, or NO_PARENT */
    uint16_t score;
} part_t;

struct boss_def
{
    const char *core_label; /* names the HUD bar */
    const part_t *parts;
    uint8_t n, core;
    uint16_t time_limit; /* frames before it escapes */
    void (*start)(void);
    /* Entry; returns true once the boss is in position to fight. */
    bool (*enter)(void);
    /* Movement and attacks while fighting. */
    void (*fight)(void);
    /* Optional: after part positions are set each frame, move some of
     * them (orbiting pods), or the boss's tile layer. */
    void (*place)(void);
    /* Optional: pick a part's image for this frame. */
    void (*frame)(uint8_t part, mode5_csprite_t *s);
    /* Optional: a part was destroyed. */
    void (*destroyed)(uint8_t part);
    /* Optional: each frame of the death throes. */
    void (*dying)(void);
    /* Optional: the boss is gone (destroyed, escaped, or the game reset). */
    void (*finish)(void);
};

/* Shared with the boss scripts. */
extern uint16_t boss_t;           /* frames since the boss appeared */
extern int16_t boss_x, boss_y;    /* Q4 center, game space */
extern int16_t part_px[BOSS_PARTS_MAX], part_py[BOSS_PARTS_MAX]; /* game pixels */
extern uint16_t part_hp[BOSS_PARTS_MAX];
extern bool part_dead[BOSS_PARTS_MAX];
extern uint8_t part_aim[BOSS_PARTS_MAX]; /* turret frame, 0 = down, clockwise */
extern uint8_t boss_destruction;  /* percent of the boss destroyed */
extern bool boss_escaped;

void boss_reset(void);
void boss_start(const boss_def_t *def);
void boss_update(void);
void boss_damage(uint8_t part, uint8_t dmg);
bool boss_done(void);
bool boss_fighting(void);
void boss_hud(bool force);
/* Turn a turret part toward the player; returns the angle to the player. */
uint8_t boss_aim(uint8_t part);

#endif
