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
#define NO_SOCKET 0xFF

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

/*
 * Scripted bosses. A boss_def_t with no fight() is run from tables instead
 * of code: a motion_t for how it moves, and a pattern_t per attack.
 */
enum
{
    PT_AIM,    /* a fan of count, step apart, aimed at the player */
    PT_DOWN,   /* a fan of count, step apart, straight down */
    PT_RING,   /* count evenly around, turning step each time */
    PT_SPIRAL, /* count arms, turning step each time: fire it often */
    PT_SPAWN,  /* launch an enemy: type arg, color step */
};

enum
{
    WHEN_ALWAYS,
    WHEN_ANGRY,  /* the core is below half its hit points */
    WHEN_DEAD,   /* part arg (or, for PT_SPAWN, the core angry) is destroyed */
    WHEN_LATE,   /* angry, or a minute in */
};

typedef struct
{
    uint8_t part;   /* the part that fires; nothing once it is destroyed */
    uint8_t kind;   /* PT_ */
    uint8_t bullet; /* BK_ */
    uint8_t speed;
    uint8_t period; /* frames between volleys */
    uint8_t phase;  /* frame within the period it fires on */
    uint8_t count;
    uint8_t step;   /* angle step (256 to a turn) */
    uint8_t when;   /* WHEN_ */
    uint8_t arg;
} pattern_t;

typedef struct
{
    uint8_t enter_speed; /* Q4 pixels a frame, coming down from the top */
    uint8_t y;           /* where it fights, game pixels */
    uint8_t amp_x, amp_y; /* hover: sine sway in pixels... */
    uint8_t rate_x, rate_y; /* ...at boss_t >> rate (0 = fastest) */
    /* Parts orbit_first to orbit_first + orbit_n - 1 circle the center on
     * an rx by ry ellipse, faster as they fall. orbit_n 0 for none. */
    uint8_t orbit_first, orbit_n, orbit_rx, orbit_ry;
    /* Parts chain_first to chain_first + chain_n - 1 trail the center one
     * after another, chain_gap frames apart, like a snake's body: the last
     * of them nearest the center. chain_n 0 for none; chain_n * chain_gap
     * up to 127 frames. */
    uint8_t chain_first, chain_n, chain_gap;
} motion_t;

/*
 * A boss drawn in plane 1's tile layer (bg_layer_show), centered on the
 * boss and moving with it. Destroyed parts leave craters in its map.
 */
typedef struct
{
    uint16_t map, tiles; /* XRAM addresses */
    uint8_t w, h;        /* in tiles */
    const uint8_t (*socket)[2]; /* each part's top left tile: column, row,
                                 * or NO_SOCKET for one not on the hull */
    uint8_t crater;      /* tile written over a one-tile part */
    uint8_t crater_big;  /* the first of four, over a larger part */
} hull_t;

struct boss_def
{
    const char *core_label; /* names the HUD bar */
    uint16_t portrait;      /* XRAM address of its 16x16 HUD portrait */
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
    /* A scripted boss (no enter or fight): its motion and attacks. */
    const motion_t *motion;
    const pattern_t *patterns;
    uint8_t n_patterns;
    /* Optional: the boss's body is a tile map. */
    const hull_t *hull;
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
