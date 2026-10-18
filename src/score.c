#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "game.h"
#include "hud.h"
#include "score.h"
#include "sprites.h"

uint8_t score_d[SCORE_DIGITS];
uint8_t hiscore_d[SCORE_DIGITS] = {0, 0, 5, 0, 0, 0, 0};

uint8_t chain_level;
uint8_t chain_max;
uint8_t chain_color;
uint8_t chain_count;
uint16_t chain_last_bonus;
uint8_t score_extends;

/* The next extend, in tens of thousands: 50,000, 100,000, then every 100,000. */
#define EXTEND_FIRST 5
#define EXTEND_EVERY 10
static uint16_t next_extend;

static bool score_dirty, chain_dirty;

/* The chain's bonus: 100, 200, 500, then 250 more a step, up to 10,000. */
static uint16_t chain_bonus(uint8_t level)
{
    if (level <= 2)
        return level * 100;
    if (level >= 41)
        return 10000;
    return 500 + (uint16_t)(level - 3) * 250;
}

void score_reset(void)
{
    memset(score_d, 0, sizeof(score_d));
    chain_level = 0;
    chain_max = 0;
    chain_count = 0;
    chain_color = COL_GRAY;
    chain_last_bonus = 0;
    score_extends = 0;
    next_extend = EXTEND_FIRST;
    score_dirty = chain_dirty = true;
}

void score_add(uint16_t pts)
{
    static const uint16_t pow10[5] = {10000, 1000, 100, 10, 1};
    uint8_t add[5];
    for (uint8_t i = 0; i < 5; i++) {
        uint8_t d = 0;
        while (pts >= pow10[i]) {
            pts -= pow10[i];
            d++;
        }
        add[i] = d;
    }
    uint8_t carry = 0;
    for (int8_t i = SCORE_DIGITS - 1; i >= 0; i--) {
        int8_t ai = (int8_t)(i - (SCORE_DIGITS - 5));
        uint8_t v = (uint8_t)(score_d[i] + carry + (ai >= 0 ? add[ai] : 0));
        carry = v >= 10;
        score_d[i] = carry ? (uint8_t)(v - 10) : v;
    }
    if (carry)
        memset(score_d, 9, sizeof(score_d)); /* counter stop */
    uint16_t ten_k = (uint16_t)(score_d[0] * 100 + score_d[1] * 10 + score_d[2]);
    while (next_extend <= ten_k) {
        next_extend = next_extend < EXTEND_EVERY ? EXTEND_EVERY : next_extend + EXTEND_EVERY;
        score_extends++;
    }
    if (memcmp(score_d, hiscore_d, SCORE_DIGITS) > 0)
        memcpy(hiscore_d, score_d, SCORE_DIGITS);
    score_dirty = true;
}

/*
 * Three of one color in a row completes a set and pays a bonus. Every
 * further kill of that color pays the next bonus up. A new color starts a
 * new set: the chain carries on if the run before it was complete (three
 * or more), and breaks if it was cut short.
 */
uint16_t chain_kill(uint8_t color)
{
    if (color == COL_GRAY)
        return 0;
    chain_dirty = true;
    if (chain_count && color == chain_color) {
        if (chain_count < 3)
            chain_count++;
    } else {
        if (chain_count && chain_count < 3)
            chain_level = 0; /* broken mid-set */
        chain_color = color;
        chain_count = 1;
    }
    if (chain_count < 3)
        return 0;
    if (chain_level < 99)
        chain_level++;
    if (chain_level > chain_max)
        chain_max = chain_level;
    chain_last_bonus = chain_bonus(chain_level);
    score_add(chain_last_bonus);
    return chain_last_bonus;
}

void chain_break(void)
{
    chain_level = 0;
    chain_count = 0;
    chain_color = COL_GRAY;
    chain_last_bonus = 0;
    chain_dirty = true;
}

/* Seven big digits (HUD sprites) across the left panel, at canvas row y. */
static void big_digits(uint8_t first, const uint8_t *d, int16_t y, uint8_t set)
{
    for (uint8_t i = 0; i < SCORE_DIGITS; i++) {
        mode5_csprite_t *s = &hspr[first + i];
        spr_setup(s, XRAM_IMG_DIGITS + (uint16_t)(d[i] + set) * DIGITS_FRAME_BYTES, PAL_ADDR(PAL_SHOTS), WH(8, 16), OPT4);
        s->x_pos_px = (int16_t)(i * 8);
        s->y_pos_px = y;
    }
}

void score_hud_hiscore(void)
{
    big_digits(HSPR_HI, hiscore_d, 6 * 8, DIGITS_GOLD);
}

void score_hud(bool force)
{
    /* Score in cyan on rows 2-3, high score in gold on rows 6-7. */
    if (score_dirty || force) {
        score_dirty = false;
        big_digits(HSPR_SCORE, score_d, 2 * 8, 0);
        score_hud_hiscore();
    }
    if (chain_dirty || force) {
        chain_dirty = false;
        hud_text(0, 10, "x", HC_GRAY, HC_PANEL);
        hud_u16(1, 10, chain_level, 2, chain_level ? HC_WHITE : HC_DIM, HC_PANEL);
        /* Three gems (HUD sprites), lit in the run's color as it builds. */
        static const uint8_t gem_pal[4] = {PAL_ENEMY_RED, PAL_ENEMY_BLUE, PAL_ENEMY_YELLOW, PAL_ENEMY_GRAY};
        for (uint8_t i = 0; i < 3; i++) {
            bool lit = i < chain_count;
            mode5_csprite_t *s = &hspr[HSPR_GEM + i];
            spr_setup(s, XRAM_IMG_GEM + (lit ? GEM_FRAME_BYTES : 0), PAL_ADDR(lit ? gem_pal[chain_color] : PAL_ENEMY_GRAY),
                      WH(8, 8), OPT4);
            s->x_pos_px = (int16_t)((1 + i * 2) * 8);
            s->y_pos_px = 11 * 8;
        }
        hud_u16(1, 13, chain_last_bonus, 5, HC_ORANGE, HC_PANEL);
    }
}

#define HISCORE_FILE "SAVE:megagammaforce.hi"

void hiscore_load(void)
{
    uint8_t d[SCORE_DIGITS];
    int fd = open(HISCORE_FILE, O_RDONLY);
    if (fd < 0)
        return;
    if (read(fd, d, sizeof(d)) == sizeof(d)) {
        bool ok = true;
        for (uint8_t i = 0; i < SCORE_DIGITS; i++)
            if (d[i] > 9)
                ok = false;
        if (ok)
            memcpy(hiscore_d, d, sizeof(d));
    }
    close(fd);
}

void hiscore_save(void)
{
    int fd = open(HISCORE_FILE, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd < 0)
        return;
    write(fd, hiscore_d, sizeof(hiscore_d));
    close(fd);
}
