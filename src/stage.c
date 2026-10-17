#include "bg.h"
#include "boss.h"
#include "enemies.h"
#include "game.h"
#include "hud.h"
#include "music.h"
#include "sfx.h"
#include "stage.h"
#include "gen/stage1_ground.h"

#define R COL_RED
#define B COL_BLUE
#define Y COL_YELLOW
#define G COL_GRAY

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

/* Colors come in threes so every formation can complete a chain set. */
static const wave_t stage1[] = {
    {120, ET_DART, R, 40, 0, 3, 16, 110},
    {220, ET_DART, B, 150, 0, 3, 16, 110},
    {330, ET_SAUCER, Y, 60, 0, 3, 22, 40},
    {430, ET_SAUCER, R, 130, 0, 3, 22, 40},
    {540, ET_DART, B, 24, 24, 3, 12, 90},
    {560, ET_DART, B, 168, -24, 3, 12, 90},
    {680, ET_DRONE, Y, 50, 0, 1, 1, 70},
    {700, ET_DRONE, Y, 142, 0, 1, 1, 70},
    {730, ET_DRONE, Y, 96, 0, 1, 1, 50},
    {860, ET_DART, R, 96, 0, 3, 10, 120},
    {900, ET_SAUCER, B, 50, 0, 3, 20, 30},
    {960, ET_SAUCER, B, 140, 0, 3, 20, 30},
    {1060, ET_GUNSHIP, G, 96, 0, 1, 1, 60},
    {1250, ET_DART, Y, 30, 20, 3, 12, 100},
    {1300, ET_DART, R, 162, -20, 3, 12, 100},
    {1400, ET_DRONE, B, 40, 56, 3, 30, 60},
    {1550, ET_SAUCER, R, 96, 0, 3, 18, 60},
    {1650, ET_DART, B, 30, 16, 3, 10, 80},
    {1700, ET_DART, B, 162, -16, 3, 10, 80},
    {1800, ET_DRONE, R, 48, 0, 1, 1, 80},
    {1820, ET_DRONE, R, 144, 0, 1, 1, 80},
    {1840, ET_DRONE, R, 96, 0, 1, 1, 60},
    {1950, ET_GUNSHIP, G, 60, 0, 1, 1, 50},
    {1990, ET_GUNSHIP, G, 132, 0, 1, 1, 80},
    {2250, ET_SAUCER, Y, 60, 0, 3, 18, 40},
    {2280, ET_SAUCER, Y, 130, 0, 3, 18, 40},
    {2400, ET_DART, R, 20, 30, 6, 8, 100},
    {2500, ET_DART, B, 172, -30, 6, 8, 100},
    {2600, ET_DRONE, Y, 40, 56, 3, 25, 70},
    {2750, ET_DART, R, 96, 0, 3, 10, 130},
    {2780, ET_DART, B, 60, 0, 3, 10, 130},
    {2810, ET_DART, Y, 132, 0, 3, 10, 130},
};
#define STAGE1_WAVES (sizeof(stage1) / sizeof(stage1[0]))

#define GROUPS 4
static uint8_t g_wave[GROUPS], g_left[GROUPS], g_timer[GROUPS], g_x[GROUPS];

uint8_t stage_phase;
static uint8_t next_wave, next_ground;
static uint16_t phase_timer;
static uint8_t msg_timer, popup_timer;


void stage_message(const char *line1, const char *line2, uint8_t frames)
{
    hud_clear_row(12);
    hud_clear_row(14);
    if (line1)
        hud_center(12, line1, HC_YELLOW);
    if (line2)
        hud_center(14, line2, HC_WHITE);
    msg_timer = frames;

}

void stage_chain_popup(uint16_t bonus)
{
    hud_clear_row(20);
    hud_text(11, 20, "CHAIN", HC_CYAN, HC_CLEAR);
    hud_u16(17, 20, bonus, 5, HC_YELLOW, HC_CLEAR);
    popup_timer = 70;
    sfx_player(SFX_CHAIN, SFX_PRI_EVENT);
}

void stage_reset(void)
{
    stage_phase = SP_INTRO;
    next_wave = 0;
    next_ground = 0;
    phase_timer = 0;
    popup_timer = 0;
    for (uint8_t i = 0; i < GROUPS; i++)
        g_left[i] = 0;
    stage_message("STAGE 1", "SKY FORTRESS", 150);
    music_play("ROM:Stage1.vgm");
}

static void start_group(uint8_t w)
{
    for (uint8_t i = 0; i < GROUPS; i++) {
        if (g_left[i] == 0) {
            g_wave[i] = w;
            g_left[i] = stage1[w].count;
            g_timer[i] = 0;
            g_x[i] = stage1[w].x;
            return;
        }
    }
}

static void run_groups(void)
{
    for (uint8_t i = 0; i < GROUPS; i++) {
        if (g_left[i] == 0)
            continue;
        if (g_timer[i]) {
            g_timer[i]--;
            continue;
        }
        const wave_t *w = &stage1[g_wave[i]];
        int16_t y = w->type == ET_GUNSHIP ? -20 : -12;
        enemy_spawn(w->type, w->color, g_x[i], y, w->param);
        g_x[i] = (uint8_t)(g_x[i] + w->dx);
        g_timer[i] = w->gap;
        g_left[i]--;
    }
}

void stage_update(uint8_t scroll)
{
    (void)scroll;
    phase_timer++;
    if (msg_timer && --msg_timer == 0) {
        hud_clear_row(12);
        hud_clear_row(14);
    }
    if (popup_timer && --popup_timer == 0)
        hud_clear_row(20);

    /* Ground turrets come with the map, whatever the phase. */
    while (next_ground < STAGE1_GROUND_N) {
        uint16_t gy = stage1_ground[next_ground][0];
        if (bg_distance + PF_H + 8 < gy)
            break;
        int16_t sy = (int16_t)(bg_distance + PF_H) - (int16_t)gy;
        enemy_spawn(ET_TURRET, (uint8_t)(next_ground % 3), (int16_t)stage1_ground[next_ground][1], sy, 0);
        next_ground++;
    }

    switch (stage_phase) {
    case SP_INTRO:
        if (phase_timer > 60) {
            stage_phase = SP_WAVES;
            phase_timer = 0;
        }
        break;
    case SP_WAVES:
        while (next_wave < STAGE1_WAVES && bg_distance >= stage1[next_wave].at)
            start_group(next_wave++);
        run_groups();
        if (bg_looping && next_wave >= STAGE1_WAVES) {
            stage_phase = SP_WARNING;
            phase_timer = 0;
            music_stop();
            stage_message("WARNING", "GAMMA CORE APPROACHING", 200);
            sfx_enemy(SFX_WARNING, SFX_PRI_TOP);
        }
        break;
    case SP_WARNING:
        run_groups();
        if (phase_timer == 100)
            sfx_enemy(SFX_WARNING, SFX_PRI_TOP);
        if (phase_timer & 16)
            hud_center(12, "WARNING", HC_RED);
        else
            hud_center(12, "WARNING", HC_YELLOW);
        if (phase_timer > 200) {
            stage_phase = SP_BOSS;
            phase_timer = 0;
            msg_timer = 0;
            hud_clear_row(12);
            hud_clear_row(14);
            boss_start();
            music_play("ROM:Boss.vgm");
        }
        break;
    case SP_BOSS:
        if (boss_done()) {
            stage_phase = SP_CLEAR;
            phase_timer = 0;
        }
        break;
    case SP_CLEAR:
        break;
    }
}
