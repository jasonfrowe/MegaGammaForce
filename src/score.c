#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "game.h"
#include "hud.h"
#include "score.h"

uint8_t score_d[SCORE_DIGITS];
uint8_t hiscore_d[SCORE_DIGITS] = {0, 0, 5, 0, 0, 0, 0};

uint8_t chain_level;
uint8_t chain_max;
uint8_t chain_color;
uint8_t chain_count;
uint16_t chain_last_bonus;

static bool score_dirty, chain_dirty;

/* 100, 200, 400 ... doubling, capped at 10000. */
static const uint16_t chain_bonus[] = {100, 200, 400, 800, 1600, 3200, 6400, 10000};

void score_reset(void)
{
    memset(score_d, 0, sizeof(score_d));
    chain_level = 0;
    chain_max = 0;
    chain_count = 0;
    chain_color = COL_GRAY;
    chain_last_bonus = 0;
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
    if (memcmp(score_d, hiscore_d, SCORE_DIGITS) > 0)
        memcpy(hiscore_d, score_d, SCORE_DIGITS);
    score_dirty = true;
}

uint16_t chain_kill(uint8_t color)
{
    if (color == COL_GRAY)
        return 0;
    chain_dirty = true;
    if (chain_count == 0 || color == chain_color) {
        chain_color = color;
        if (++chain_count < 3)
            return 0;
        chain_count = 0;
        if (chain_level < 99)
            chain_level++;
        if (chain_level > chain_max)
            chain_max = chain_level;
        uint8_t i = (uint8_t)(chain_level - 1);
        if (i > 7)
            i = 7;
        chain_last_bonus = chain_bonus[i];
        score_add(chain_last_bonus);
        return chain_last_bonus;
    }
    /* Broken: this kill starts a new set. */
    chain_level = 0;
    chain_color = color;
    chain_count = 1;
    return 0;
}

static const uint8_t color_hc[4] = {HC_RED, HC_BLUE, HC_YELLOW, HC_GRAY};

void score_hud(bool force)
{
    if (score_dirty || force) {
        score_dirty = false;
        hud_digits(0, 3, score_d, SCORE_DIGITS, HC_WHITE);
        hud_digits(0, 6, hiscore_d, SCORE_DIGITS, HC_YELLOW);
    }
    if (chain_dirty || force) {
        chain_dirty = false;
        hud_text(0, 10, "x", HC_GRAY, HC_PANEL);
        hud_u16(1, 10, chain_level, 2, chain_level ? HC_WHITE : HC_DIM, HC_PANEL);
        /* Three pips: filled in the set's color as the set builds. */
        uint8_t fg = color_hc[chain_color];
        for (uint8_t i = 0; i < 3; i++) {
            bool lit = i < chain_count;
            hud_fill((uint8_t)(1 + i * 2), 11, 1, lit ? 0xFE : 0xF9, lit ? fg : HC_DIM, HC_PANEL);
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
