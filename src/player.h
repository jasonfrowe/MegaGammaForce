#ifndef MGF_PLAYER_H
#define MGF_PLAYER_H

#include <stdbool.h>
#include <stdint.h>

/* Guns are indexed by the A/B/C buttons held (A=1, B=2, C=4). The swords
 * have buttons of their own. */
enum
{
    WP_NONE = 0,
    WP_VULCAN = 1,  /* A */
    WP_HOMING = 2,  /* B */
    WP_PLASMA = 3,  /* A+B homing plasma */
    WP_SPREAD = 4,  /* C */
    WP_WIDE = 5,    /* A+C wide shot */
    WP_MISSILE = 6, /* B+C homing missiles */
    WP_ALL = 7,     /* A+B+C: fires the Vulcan */
    WP_SWORD = 8,
    WP_HYPER = 9,
};

#define SWORD_GAUGE_MAX 48

enum
{
    PL_ALIVE,
    PL_DEAD,
};

extern int16_t pl_x, pl_y;   /* Q4 game space, ship center */
extern uint8_t pl_bx, pl_by; /* ship center in bullet space, for hit tests */
extern uint8_t pl_state;
extern uint8_t pl_lives;
extern uint8_t pl_weapon;
extern uint8_t sword_gauge;
extern bool sword_active;
extern uint8_t sword_x0, sword_x1, sword_y0, sword_y1; /* bullet space */
extern uint8_t hyper_timer;

void player_reset(void);
void player_respawn(void);
void player_update(bool controls);
void player_hit(void);
bool player_vulnerable(void);
void sword_absorb(void);
void player_hud(bool force);
void player_hide(void);

#endif
