#include <fcntl.h>
#include <rp6502.h>
#include <unistd.h>
#include "bg.h"
#include "boss.h"
#include "bosses.h"
#include "enemies.h"
#include "game.h"
#include "hud.h"
#include "music.h"
#include "sfx.h"
#include "sprites.h"
#include "stage.h"
#include "xram.h"
#include "gen/stage1.h"
#include "gen/stage1_data.h"
#include "gen/stage2.h"
#include "gen/stage2_data.h"
#include "gen/stage3.h"
#include "gen/stage3_data.h"

#define R COL_RED
#define B COL_BLUE
#define Y COL_YELLOW
#define G COL_GRAY

#define N(a) ((uint8_t)(sizeof(a) / sizeof((a)[0])))

/* Colors come in threes so every formation can complete a chain set. */
static const wave_t s1_waves[] = {
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

/* Stage 2: wasps, tanks on the roads, and towers to dodge. */
static const wave_t s2_waves[] = {
    {140, ET_WASP, R, 50, 0, 3, 14, 30},
    {220, ET_WASP, R, 140, 0, 3, 14, 30},
    {330, ET_DART, B, 30, 26, 3, 12, 100},
    {390, ET_DART, B, 162, -26, 3, 12, 100},
    {500, ET_SAUCER, Y, 96, 0, 3, 20, 50},
    {620, ET_WASP, B, 40, 20, 3, 10, 24},
    {660, ET_WASP, Y, 150, -20, 3, 10, 24},
    {780, ET_DRONE, R, 60, 0, 1, 1, 70},
    {800, ET_DRONE, R, 132, 0, 1, 1, 70},
    {820, ET_DRONE, R, 96, 0, 1, 1, 50},
    {940, ET_GUNSHIP, G, 96, 0, 1, 1, 60},
    {1120, ET_WASP, Y, 30, 0, 3, 12, 20},
    {1140, ET_WASP, Y, 162, 0, 3, 12, 20},
    {1260, ET_SAUCER, B, 50, 0, 3, 18, 40},
    {1290, ET_SAUCER, R, 140, 0, 3, 18, 40},
    {1410, ET_DART, Y, 20, 30, 6, 8, 110},
    {1530, ET_DRONE, B, 40, 56, 3, 30, 60},
    {1660, ET_WASP, R, 96, 0, 6, 10, 40},
    {1790, ET_GUNSHIP, G, 60, 0, 1, 1, 50},
    {1830, ET_GUNSHIP, G, 132, 0, 1, 1, 80},
    {2010, ET_DART, B, 172, -30, 6, 8, 100},
    {2110, ET_SAUCER, Y, 60, 0, 3, 18, 40},
    {2140, ET_SAUCER, Y, 130, 0, 3, 18, 40},
    {2270, ET_WASP, B, 30, 14, 3, 10, 30},
    {2300, ET_WASP, B, 160, -14, 3, 10, 30},
    {2410, ET_DRONE, Y, 40, 56, 3, 25, 70},
    {2560, ET_WASP, R, 40, 0, 3, 8, 20},
    {2570, ET_WASP, B, 96, 0, 3, 8, 20},
    {2580, ET_WASP, Y, 152, 0, 3, 8, 20},
    {2710, ET_DART, R, 96, 0, 3, 10, 130},
    {2770, ET_GUNSHIP, G, 96, 0, 1, 1, 60},
};

/* Stage 3: everything at once over the open sea, with gunboats below. */
static const wave_t s3_waves[] = {
    {120, ET_DART, Y, 30, 22, 3, 12, 100},
    {180, ET_DART, Y, 162, -22, 3, 12, 100},
    {300, ET_WASP, R, 60, 0, 3, 12, 30},
    {340, ET_WASP, R, 132, 0, 3, 12, 30},
    {460, ET_SAUCER, B, 40, 0, 3, 18, 36},
    {490, ET_SAUCER, B, 152, 0, 3, 18, 36},
    {620, ET_GUNSHIP, G, 96, 0, 1, 1, 60},
    {760, ET_DRONE, Y, 50, 46, 3, 24, 70},
    {880, ET_WASP, B, 20, 26, 6, 9, 30},
    {1000, ET_DART, R, 172, -26, 6, 8, 110},
    {1120, ET_SAUCER, Y, 96, 0, 3, 16, 60},
    {1150, ET_WASP, Y, 40, 0, 3, 10, 20},
    {1180, ET_WASP, Y, 152, 0, 3, 10, 20},
    {1320, ET_GUNSHIP, G, 50, 0, 1, 1, 50},
    {1340, ET_GUNSHIP, G, 142, 0, 1, 1, 80},
    {1500, ET_DRONE, R, 60, 0, 1, 1, 70},
    {1520, ET_DRONE, R, 132, 0, 1, 1, 70},
    {1540, ET_DRONE, R, 96, 0, 1, 1, 50},
    {1660, ET_DART, B, 30, 18, 6, 8, 120},
    {1780, ET_WASP, R, 96, 0, 6, 8, 44},
    {1900, ET_SAUCER, B, 50, 0, 3, 16, 40},
    {1930, ET_SAUCER, B, 140, 0, 3, 16, 40},
    {2060, ET_DRONE, Y, 40, 56, 3, 22, 60},
    {2200, ET_GUNSHIP, G, 96, 0, 1, 1, 60},
    {2340, ET_WASP, R, 30, 0, 3, 8, 24},
    {2350, ET_WASP, B, 96, 0, 3, 8, 24},
    {2360, ET_WASP, Y, 162, 0, 3, 8, 24},
    {2500, ET_DART, Y, 20, 30, 6, 7, 110},
    {2600, ET_DART, R, 172, -30, 6, 7, 110},
    {2720, ET_GUNSHIP, G, 60, 0, 1, 1, 50},
    {2750, ET_GUNSHIP, G, 132, 0, 1, 1, 70},
};

static const stage_def_t stage_defs[] = {
    {
        .bank = "ROM:stage1.bin",
        .name = "STAGE 1",
        .title = "SKY FORTRESS",
        .warning = "GAMMA CORE APPROACHING",
        .music = "ROM:Stage1.vgm",
        .boss_music = "ROM:Boss.vgm",
        .speed = 160,
        .gspr_n = 8,
        .tiles = S1_TILES,
        .map = S1_MAP,
        .map_rows = S1_MAP_ROWS,
        .loop_rows = S1_MAP_LOOP_ROWS,
        .waves = s1_waves,
        .n_waves = N(s1_waves),
        .ground = s1_ground,
        .n_ground = S1_GROUND_N,
        .boss = &boss1,
    },
    {
        .bank = "ROM:stage2.bin",
        .name = "STAGE 2",
        .title = "NEON SPIRES",
        .warning = "SPIRE WARDEN APPROACHING",
        .music = "ROM:Stage2.vgm",
        .boss_music = "ROM:Boss.vgm",
        .speed = 160,
        .gspr_n = 12,
        .tiles = S2_TILES,
        .map = S2_MAP,
        .map_rows = S2_MAP_ROWS,
        .loop_rows = S2_MAP_LOOP_ROWS,
        .sky_tiles = S2_SKY_TILES,
        .sky_map = S2_SKY_MAP,
        .sky_rows = S2_SKY_ROWS,
        .sky_speed = S2_SKY_SPEED,
        .sky_solid = s2_sky_solid,
        .waves = s2_waves,
        .n_waves = N(s2_waves),
        .ground = s2_ground,
        .n_ground = S2_GROUND_N,
        .boss = &boss2,
    },
    {
        .bank = "ROM:stage3.bin",
        .name = "STAGE 3",
        .title = "IRON TIDE",
        .warning = "BATTLESHIP LEVIATHAN",
        .music = "ROM:Stage3.vgm",
        .boss_music = "ROM:Boss3.vgm",
        .speed = 160,
        .gspr_n = 16,
        .tiles = S3_TILES,
        .map = S3_MAP,
        .map_rows = S3_MAP_ROWS,
        .loop_rows = S3_MAP_LOOP_ROWS,
        .sky_tiles = S3_SKY_TILES,
        .sky_map = S3_SKY_MAP,
        .sky_rows = S3_SKY_ROWS,
        .sky_speed = S3_SKY_SPEED,
        .sky_solid = 0, /* clouds only: nothing to crash into */
        .waves = s3_waves,
        .n_waves = N(s3_waves),
        .ground = s3_ground,
        .n_ground = S3_GROUND_N,
        .boss = &boss3,
    },
};
_Static_assert(N(stage_defs) == STAGE_COUNT, "a stage in the art has no stage_defs entry, or the other way");

#define GROUPS 4
static uint8_t g_wave[GROUPS], g_left[GROUPS], g_timer[GROUPS], g_x[GROUPS];

uint8_t stage_phase;
uint8_t stage_num;
const stage_def_t *stage = &stage_defs[0];
volatile uint8_t first_stage; /* poked by test scripts */
static uint8_t loaded = 0xFF; /* the stage whose bank is in XRAM */
static uint8_t next_wave, next_ground;
static uint16_t phase_timer;
static uint8_t msg_timer, popup_timer;

static void curtain(void)
{
    for (uint8_t r = 0; r < HUD_ROWS; r++)
        hud_fill(HUD_PF_COL, r, 24, ' ', HC_BLACK, HC_BLACK);
}

void stage_load(uint8_t n)
{
    stage_num = n;
    stage = &stage_defs[n];
    if (loaded != n) {
        /* The HUD is in front of every plane, so a black playfield hides
         * the tiles and sprites while their data is replaced. */
        curtain();
        int fd = open(stage->bank, O_RDONLY);
        if (fd >= 0) {
            read_xram(PAL_ADDR(PAL_STAGE), PAL_STAGE_N * sizeof(palette_t), fd);
            uint16_t addr = XRAM_BANK;
            int got;
            while ((got = read_xram(addr, 0x2000, fd)) > 0)
                addr += (uint16_t)got;
            close(fd);
        }
        bg_capture_palettes();
        loaded = n;
    }
    spr_ground_count(stage->gspr_n);
    bg_load();
}

bool stage_is_last(void)
{
    return stage_num + 1 >= STAGE_COUNT;
}

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
    stage_message(stage->name, stage->title, 150);
    music_play(stage->music);
}

static void start_group(uint8_t w)
{
    for (uint8_t i = 0; i < GROUPS; i++) {
        if (g_left[i] == 0) {
            g_wave[i] = w;
            g_left[i] = stage->waves[w].count;
            g_timer[i] = 0;
            g_x[i] = stage->waves[w].x;
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
        const wave_t *w = &stage->waves[g_wave[i]];
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

    /* Ground units come with the map, whatever the phase. */
    while (next_ground < stage->n_ground) {
        const ground_t *g = &stage->ground[next_ground];
        if (bg_distance + PF_H + 16 < g->y)
            break;
        int16_t sy = (int16_t)(bg_distance + PF_H) - (int16_t)g->y;
        enemy_spawn(g->type, g->color, g->x, sy, g->param);
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
        while (next_wave < stage->n_waves && bg_distance >= stage->waves[next_wave].at)
            start_group(next_wave++);
        run_groups();
        if (bg_looping && next_wave >= stage->n_waves) {
            stage_phase = SP_WARNING;
            phase_timer = 0;
            music_stop();
            stage_message("WARNING", stage->warning, 200);
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
            boss_start(stage->boss);
            music_play(stage->boss_music);
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
