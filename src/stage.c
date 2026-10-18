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
#include "score.h"
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
#include "gen/stage4.h"
#include "gen/stage4_data.h"
#include "gen/stage5.h"
#include "gen/stage5_data.h"
#include "gen/stage6.h"
#include "gen/stage6_data.h"
#include "gen/stage7.h"
#include "gen/stage7_data.h"
#include "gen/stage8.h"
#include "gen/stage8_data.h"
#include "gen/stage9.h"
#include "gen/stage9_data.h"

#define N(a) ((uint8_t)(sizeof(a) / sizeof((a)[0])))

/* Stage 9's boss rush: the Spire Warden, the Dune Wyrm, the Halo Station. */
static const uint8_t rush9[] = {1, 3, 5};

static const stage_def_t stage_defs[] = {
    {
        .bank = "ROM:stage1.bin",
        .boss_bank = "ROM:boss1.bin",
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
        .boss = &boss1,
    },
    {
        .bank = "ROM:stage2.bin",
        .boss_bank = "ROM:boss2.bin",
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
        .sky_clear = S2_SKY_CLEAR,
        .sky_fine = S2_SKY_FINE,
        .boss = &boss2,
    },
    {
        .bank = "ROM:stage3.bin",
        .boss_bank = "ROM:boss3.bin",
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
        /* clouds only: nothing to crash into */
        .boss = &boss3,
    },
    {
        .bank = "ROM:stage4.bin",
        .boss_bank = "ROM:boss4.bin",
        .name = "STAGE 4",
        .title = "SCORCHED SANDS",
        .warning = "DUNE WYRM SURFACING",
        .music = "ROM:Stage4.vgm",
        .boss_music = "ROM:Boss4.vgm",
        .speed = 160,
        .gspr_n = 12,
        .tiles = S4_TILES,
        .map = S4_MAP,
        .map_rows = S4_MAP_ROWS,
        .loop_rows = S4_MAP_LOOP_ROWS,
        .sky_tiles = S4_SKY_TILES,
        .sky_map = S4_SKY_MAP,
        .sky_rows = S4_SKY_ROWS,
        .sky_speed = S4_SKY_SPEED,
        .boss = &boss4,
    },
    {
        .bank = "ROM:stage5.bin",
        .boss_bank = "ROM:boss5.bin",
        .name = "STAGE 5",
        .title = "FROZEN STRAIT",
        .warning = "GLACIER CRAWLER ADVANCING",
        .music = "ROM:Stage5.vgm",
        .boss_music = "ROM:Boss5.vgm",
        .speed = 160,
        .gspr_n = 16,
        .tiles = S5_TILES,
        .map = S5_MAP,
        .map_rows = S5_MAP_ROWS,
        .loop_rows = S5_MAP_LOOP_ROWS,
        .sky_tiles = S5_SKY_TILES,
        .sky_map = S5_SKY_MAP,
        .sky_rows = S5_SKY_ROWS,
        .sky_speed = S5_SKY_SPEED,
        .sky_clear = S5_SKY_CLEAR,
        .sky_fine = S5_SKY_FINE,
        .boss = &boss5,
    },
    {
        .bank = "ROM:stage6.bin",
        .boss_bank = "ROM:boss6.bin",
        .name = "STAGE 6",
        .title = "ASCENT",
        .warning = "HALO STATION IN ORBIT",
        .music = "ROM:Stage6.vgm",
        .boss_music = "ROM:Boss6.vgm",
        .speed = 160,
        .gspr_n = 8,
        .tiles = S6_TILES,
        .map = S6_MAP,
        .map_rows = S6_MAP_ROWS,
        .loop_rows = S6_MAP_LOOP_ROWS,
        .sky_tiles = S6_SKY_TILES,
        .sky_map = S6_SKY_MAP,
        .sky_rows = S6_SKY_ROWS,
        .sky_speed = S6_SKY_SPEED,
        .sky_clear = S6_SKY_CLEAR,
        .sky_fine = S6_SKY_FINE,
        /* The ocean and clouds darken as the ship climbs, until the limb. */
        .fade_first = S6_FADE_FIRST,
        .fade_n = S6_FADE_N,
        .fade_from = 200,
        .fade_to = 1600,
        .boss = &boss6,
    },
    {
        .bank = "ROM:stage7.bin",
        .boss_bank = "ROM:boss7.bin",
        .name = "STAGE 7",
        .title = "SEA OF TRANQUILITY",
        .warning = "MASS DRIVER CHARGING",
        .music = "ROM:Stage7.vgm",
        .boss_music = "ROM:Boss7.vgm",
        .speed = 160,
        .gspr_n = 12,
        .tiles = S7_TILES,
        .map = S7_MAP,
        .map_rows = S7_MAP_ROWS,
        .loop_rows = S7_MAP_LOOP_ROWS,
        .boss = &boss7,
    },
    {
        .bank = "ROM:stage8.bin",
        .boss_bank = "ROM:boss8.bin",
        .name = "STAGE 8",
        .title = "DEEP FIELD",
        .warning = "HIVE MOTHERSHIP AWAKENS",
        .music = "ROM:Stage8.vgm",
        .boss_music = "ROM:Boss8.vgm",
        .speed = 160,
        .gspr_n = 8,
        .tiles = S8_TILES,
        .map = S8_MAP,
        .map_rows = S8_MAP_ROWS,
        .loop_rows = S8_MAP_LOOP_ROWS,
        .sky_tiles = S8_SKY_TILES,
        .sky_map = S8_SKY_MAP,
        .sky_rows = S8_SKY_ROWS,
        .sky_speed = S8_SKY_SPEED,
        .sky_clear = S8_SKY_CLEAR,
        .sky_fine = S8_SKY_FINE,
        .boss = &boss8,
    },
    {
        .bank = "ROM:stage9.bin",
        .boss_bank = "ROM:boss9.bin",
        .name = "STAGE 9",
        .title = "CORONA",
        .warning = "THE GUARDIANS RETURN",
        .final_warning = "GAMMA PRIME IGNITES",
        .music = "ROM:Stage9.vgm",
        .boss_music = "ROM:Final.vgm",
        .speed = 160,
        .gspr_n = 8,
        .tiles = S9_TILES,
        .map = S9_MAP,
        .map_rows = S9_MAP_ROWS,
        .loop_rows = S9_MAP_LOOP_ROWS,
        .sky_tiles = S9_SKY_TILES,
        .sky_map = S9_SKY_MAP,
        .sky_rows = S9_SKY_ROWS,
        .sky_speed = S9_SKY_SPEED,
        .flare_a = S9_FLARE_A,
        .flare_b = S9_FLARE_B,
        .rush = rush9,
        .rush_n = N(rush9),
        .boss = &boss9,
    },
};
_Static_assert(N(stage_defs) == STAGE_COUNT, "a stage in the art has no stage_defs entry, or the other way");

#define GROUPS 4
static uint8_t g_wave[GROUPS], g_left[GROUPS], g_timer[GROUPS], g_x[GROUPS];

uint8_t stage_phase;
uint8_t stage_num;
const stage_def_t *stage = &stage_defs[0];
volatile uint8_t first_stage; /* poked by test scripts */
bool stage_boss_rush;
static uint8_t loaded = 0xFF; /* the stage whose bank is in XRAM */
static uint8_t next_wave, next_ground;
static uint8_t rush_i; /* bosses of the rush begun */
static uint16_t phase_timer;
static uint8_t msg_timer, popup_timer;

static void curtain(void)
{
    for (uint8_t r = 0; r < HUD_ROWS; r++)
        hud_fill(HUD_PF_COL, r, 24, ' ', HC_BLACK, HC_BLACK);
}

/* A ROM file: npal palettes into the slots from first_pal, the rest at addr. */
static void load_file(const char *name, uint8_t first_pal, uint8_t npal, uint16_t addr)
{
    int fd = open(name, O_RDONLY);
    if (fd < 0)
        return;
    read_xram(PAL_ADDR(first_pal), npal * sizeof(palette_t), fd);
    int got;
    while ((got = read_xram(addr, 0x2000, fd)) > 0)
        addr += (uint16_t)got;
    close(fd);
}

void boss_bank_load(uint8_t n)
{
    load_file(stage_defs[n].boss_bank, PAL_LAYER, 3, XRAM_BOSS);
    bg_capture_layer();
}

/* The stage's waves and ground units, read from the front of its bank. */
static wave_t waves[WAVES_MAX];
static ground_t ground[GROUND_MAX];
static uint8_t n_waves, n_ground;

void stage_load(uint8_t n)
{
    stage_num = n;
    stage = &stage_defs[n];
    /* The HUD is in front of every plane, so a black playfield hides
     * the tiles and sprites while their data is replaced. */
    curtain();
    /* The boss file always comes fresh: a tile boss cratered its map. */
    boss_bank_load(n);
    frame_loaded = true;
    if (loaded != n) {
        load_file(stage->bank, PAL_TILES, 2, XRAM_BANK);
        n_waves = xram0_peek8(XRAM_BANK);
        n_ground = xram0_peek8(XRAM_BANK + 1);
        xram0_read(waves, XRAM_BANK + 2, n_waves * sizeof(wave_t));
        xram0_read(ground, XRAM_BANK + 2 + n_waves * sizeof(wave_t), n_ground * sizeof(ground_t));
        bg_capture_palettes();
        loaded = n;
    }
    spr_ground_count(stage->gspr_n);
    bg_load();
}

const char *stage_title(uint8_t n)
{
    return n < STAGE_COUNT ? stage_defs[n].title : "ALL NINE BOSSES";
}

/* The bosses of this stage's own rush: none in the boss rush mode. */
static uint8_t rush_n(void)
{
    return stage_boss_rush ? 0 : stage->rush_n;
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

/* "STAGE 2" in big gold letters across the playfield, doubled. */
static void banner(bool show)
{
    mode5_csprite_t *w = &hspr[HSPR_BANNER], *n = &hspr[HSPR_BANNER + 1];
    if (!show) {
        spr_hide(w);
        spr_hide(n);
        return;
    }
    /* 128 + 8 + 32 pixels wide, centered over the stage's name. */
    int16_t x = PF_X + (PF_W - 168) / 2;
    spr_setup(w, XRAM_IMG_BANNER, PAL_ADDR(PAL_SHOTS), WH(64, 16), OPT4 | MODE5_HDOUBLE | MODE5_VDOUBLE);
    w->x_pos_px = x;
    w->y_pos_px = 62;
    spr_setup(n, XRAM_IMG_BANNERNUM + stage_num * BANNERNUM_FRAME_BYTES, PAL_ADDR(PAL_SHOTS), WH(16, 16),
              OPT4 | MODE5_HDOUBLE | MODE5_VDOUBLE);
    n->x_pos_px = x + 136;
    n->y_pos_px = 62;
}

/* "WARNING" across the playfield, doubled, flashing red and gold. */
static void warning_banner(bool red)
{
    mode5_csprite_t *a = &hspr[HSPR_BANNER], *b = &hspr[HSPR_BANNER + 1];
    uint16_t pal = PAL_ADDR(red ? PAL_ENEMY_RED : PAL_SHOTS);
    int16_t x = PF_X + (PF_W - 176) / 2;
    spr_setup(a, XRAM_IMG_WARN_A, pal, WH(64, 16), OPT4 | MODE5_HDOUBLE | MODE5_VDOUBLE);
    a->x_pos_px = x;
    a->y_pos_px = 62;
    spr_setup(b, XRAM_IMG_WARN_B, pal, WH(24, 16), OPT4 | MODE5_HDOUBLE | MODE5_VDOUBLE);
    b->x_pos_px = x + 128;
    b->y_pos_px = 62;
}

void stage_reset(void)
{
    stage_phase = SP_INTRO;
    next_wave = 0;
    next_ground = 0;
    rush_i = 0;
    phase_timer = 0;
    popup_timer = 0;
    for (uint8_t i = 0; i < GROUPS; i++)
        g_left[i] = 0;
    stage_message(0, stage->title, 150);
    banner(true);
    if (stage_boss_rush) {
        /* Straight to the end of the stage, quiet, and the warning. */
        bg_skip_to_end();
        next_wave = n_waves;
        next_ground = n_ground;
        return;
    }
    music_play(stage->music);
}

static void start_group(uint8_t w)
{
    for (uint8_t i = 0; i < GROUPS; i++) {
        if (g_left[i] == 0) {
            g_wave[i] = w;
            g_left[i] = waves[w].count;
            g_timer[i] = 0;
            g_x[i] = waves[w].x;
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
        const wave_t *w = &waves[g_wave[i]];
        int16_t y = w->type == ET_GUNSHIP ? -20 : -12;
        enemy_spawn(w->type, w->color, g_x[i], y, w->param);
        g_x[i] = (uint8_t)(g_x[i] + w->dx);
        g_timer[i] = w->gap;
        g_left[i]--;
    }
}

static void warn(const char *text)
{
    stage_phase = SP_WARNING;
    phase_timer = 0;
    music_stop();
    stage_message(0, text, 200);
    warning_banner(true);
    sfx_enemy(SFX_WARNING, SFX_PRI_TOP);
}

/* The next boss: in a rush, the next of the rush (its file loaded over the
 * last one's), then the stage's own. */
static void begin_boss(void)
{
    uint8_t n = rush_i < rush_n() ? stage->rush[rush_i] : stage_num;
    if (rush_n()) {
        boss_bank_load(n);
        frame_loaded = true;
    }
    stage_phase = SP_BOSS;
    phase_timer = 0;
    boss_start(stage_defs[n].boss);
    music_play(stage_defs[n].boss_music);
    if (n != stage_num)
        stage_message(0, stage_defs[n].warning, 120);
}

/* A boss of the rush is done: its bonus now, as the results would. */
static void rush_done(void)
{
    static char line[] = "DESTROYED 000%";
    uint8_t d = boss_destruction;
    line[10] = (char)('0' + d / 100);
    line[11] = (char)('0' + d / 10 % 10);
    line[12] = (char)('0' + d % 10);
    score_add((uint16_t)d * 100 + (d >= 100 ? 10000 : 0));
    stage_message(boss_escaped ? "BOSS ESCAPED" : "BOSS DOWN", line, 150);
    music_stop();
    rush_i++;
    stage_phase = SP_RUSH;
    phase_timer = 0;
}

void stage_update(uint8_t scroll)
{
    (void)scroll;
    phase_timer++;
    if (msg_timer && --msg_timer == 0) {
        hud_clear_row(12);
        hud_clear_row(14);
        banner(false);
    }
    if (popup_timer && --popup_timer == 0)
        hud_clear_row(20);

    /* Ground units come with the map, whatever the phase. */
    while (next_ground < n_ground) {
        const ground_t *g = &ground[next_ground];
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
        while (next_wave < n_waves && bg_distance >= waves[next_wave].at)
            start_group(next_wave++);
        run_groups();
        if (bg_looping && next_wave >= n_waves)
            warn(stage->warning);
        break;
    case SP_WARNING:
        run_groups();
        if (phase_timer == 100)
            sfx_enemy(SFX_WARNING, SFX_PRI_TOP);
        if ((phase_timer & 15) == 0)
            warning_banner(!(phase_timer & 16));
        if (phase_timer > 200) {
            msg_timer = 0;
            hud_clear_row(12);
            hud_clear_row(14);
            banner(false);
            begin_boss();
        }
        break;
    case SP_BOSS:
        if (boss_done()) {
            if (rush_i < rush_n()) {
                rush_done();
            } else {
                stage_phase = SP_CLEAR;
                phase_timer = 0;
            }
        }
        break;
    case SP_RUSH:
        if (phase_timer > 180) {
            if (rush_i < rush_n())
                begin_boss();
            else
                warn(stage->final_warning);
        }
        break;
    case SP_CLEAR:
        break;
    }
}
