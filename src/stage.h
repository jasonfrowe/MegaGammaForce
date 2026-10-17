#ifndef STAGE_H
#define STAGE_H

#include <stdbool.h>
#include <stdint.h>

enum
{
    SP_INTRO,
    SP_WAVES,
    SP_WARNING,
    SP_BOSS,
    SP_CLEAR,
};

/* A group of enemies: count of them, gap frames apart, each dx further across. */
typedef struct
{
    uint16_t at; /* scroll distance in pixels */
    uint8_t type;
    uint8_t color;
    uint8_t x;
    int8_t dx;
    uint8_t count;
    uint8_t gap;
    uint8_t param;
} wave_t;

/* A ground unit placed on the map: y is its center measured up from the
 * bottom of the map, and it spawns as that row scrolls onto the screen. */
typedef struct
{
    uint16_t y;
    uint8_t x;
    uint8_t type;
    uint8_t color;
    uint8_t param;
} ground_t;

typedef struct boss_def boss_def_t;

typedef struct
{
    const char *bank; /* ROM file: the stage's palettes, then its bank */
    const char *name, *title, *warning;
    const char *music, *boss_music;
    uint8_t speed;  /* ground scroll, pixels per frame in 1/256 */
    uint8_t gspr_n; /* ground sprites this stage shows, up to GSPR_MAX */
    /* Ground (plane 0): offsets in the bank, rows in the map. */
    uint16_t tiles, map, map_rows, loop_rows;
    /* Parallax sky (plane 1), or sky_rows 0 for none. It scrolls
     * sky_speed / 4 times as fast as the ground, and stops at its top row. */
    uint16_t sky_tiles, sky_map, sky_rows;
    uint8_t sky_speed;
    const uint8_t *sky_solid; /* a bit per tile: crashes the ship */
    const wave_t *waves;
    uint8_t n_waves;
    const ground_t *ground;
    uint8_t n_ground;
    const boss_def_t *boss;
} stage_def_t;

extern uint8_t stage_phase;
extern uint8_t stage_num;           /* 0-based */
extern const stage_def_t *stage;    /* stage_defs[stage_num] */
extern volatile uint8_t first_stage; /* where a new game starts, for testing */

/* Make stage n current and load its bank, behind a black curtain. */
void stage_load(uint8_t n);
bool stage_is_last(void);
void stage_reset(void);
/* Runs the script for this frame; scroll is the pixels scrolled. */
void stage_update(uint8_t scroll);
void stage_chain_popup(uint16_t bonus);
/* Big centered message in the playfield for a time; NULL clears it. */
void stage_message(const char *line1, const char *line2, uint8_t frames);

#endif
