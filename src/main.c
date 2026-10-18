#include <rp6502.h>
#include "bg.h"
#include "boss.h"
#include "bullets.h"
#include "enemies.h"
#include "fx.h"
#include "game.h"
#include "hud.h"
#include "input.h"
#include "music.h"
#include "player.h"
#include "score.h"
#include "sfx.h"
#include "shots.h"
#include "sprites.h"
#include "stage.h"
#include "targets.h"
#include "util.h"
#include "xram.h"
#include "gen/stage1.h"

uint8_t frame_count;
bool frame_loaded;

/* Frames the game logic overran, for profiling from the emulator. */
volatile uint16_t lag_frames;

enum
{
    GS_TITLE,
    GS_PLAY,
    GS_RESULTS,
    GS_GAMEOVER,
};

static uint8_t gstate;
static uint16_t gtimer;
static bool paused;
static uint16_t results_bonus;
static bool all_clear; /* the results of the last stage, then the ending */

static void clear_playfield(void)
{
    for (uint8_t r = 0; r < HUD_ROWS; r++)
        hud_clear_row(r);
}

static void reset_world(void)
{
    spr_hide_all();
    bullets_reset();
    shots_reset();
    enemies_reset();
    fx_reset();
    boss_reset();
    tgt_clear();
}

/* A double rule across a panel, between its sections. */
static void rule(uint8_t col, uint8_t row)
{
    hud_fill(col, row, 7, 0xCD, HC_BORDER, HC_PANEL);
}

/*
 * The side panels. Big digits, icons, gems and the boss portrait are HUD
 * sprites on plane 2 (score.c, player.c, boss.c); the text is plane 2's
 * character layer.
 *
 *   left                    right
 *    1  SCORE                2-3  [weapon icon] LV, progress
 *    2-3  score digits       4-5  weapon name
 *    5  HISCORE              6    SWORD, 7 gauge, 8-9 hyper sign
 *    6-7  high score         10   LIVES, 12 "03 x" and a ship
 *    8  ===                  13   ===
 *    9  CHAIN, 10 x9,        14-15 [portrait] BOSS
 *       11 gems              16   DESTROY, 17 percent
 *    12 BONUS, 13 value      19   core label, 20 its health
 *    14 ===
 *    26 STAGE, 27-28 number
 */
static void draw_labels(void)
{
    hud_frame();
    hud_text(0, 1, "SCORE", HC_CYAN, HC_PANEL);
    hud_text(0, 5, stage_boss_rush ? "RUSH HI" : "HISCORE", HC_CYAN, HC_PANEL);
    rule(0, 8);
    hud_text(0, 9, "CHAIN", HC_CYAN, HC_PANEL);
    hud_text(0, 12, "BONUS", HC_GRAY, HC_PANEL);
    rule(0, 14);
    hud_text(0, 26, "STAGE", HC_GRAY, HC_PANEL);
    mode5_csprite_t *s = &hspr[HSPR_STAGE];
    spr_setup(s, XRAM_IMG_DIGITS + (uint16_t)(stage_num + 1 + DIGITS_GOLD) * DIGITS_FRAME_BYTES, PAL_ADDR(PAL_SHOTS),
              WH(8, 16), OPT4);
    s->x_pos_px = 6 * 8;
    s->y_pos_px = 26 * 8 - 4;
    hud_text(33, 6, "SWORD", HC_CYAN, HC_PANEL);
    hud_text(33, 10, "LIVES", HC_CYAN, HC_PANEL);
    rule(33, 13);
    score_hud(true);
    player_hud(true);
}

/* The title's stage select: left and right choose where a game starts. */
static void title_stage(void)
{
    char t[] = "<  STAGE 1  >";
    t[9] = (char)('1' + first_stage);
    hud_clear_row(16);
    hud_center(16, first_stage < STAGE_COUNT ? t : "<  BOSS RUSH  >", HC_CYAN);
    hud_clear_row(17);
    hud_center(17, stage_title(first_stage), HC_WHITE);
    /* Each mode shows its own high score. */
    hiscore_select(first_stage >= STAGE_COUNT);
    hud_text(0, 5, first_stage >= STAGE_COUNT ? "RUSH HI" : "HISCORE", HC_CYAN, HC_PANEL);
    score_hud_hiscore();
}

static void enter_title(void)
{
    gstate = GS_TITLE;
    gtimer = 0;
    stage_boss_rush = false;
    reset_world();
    stage_load(0); /* the title flies over stage 1, and its bank has the logo */
    bg_reset();
    bg_set_speed(96);
    hud_frame();
    clear_playfield();
    for (uint8_t k = 0; k < 3; k++) {
        mode5_csprite_t *s = &spr[SPR_BOSS + k];
        spr_setup(s, XRAM_BANK + S1_LOGO + k * S1_LOGO_FRAME_BYTES, PAL_ADDR(PAL_BOSS), WH(64, 64), OPT4);
        s->x_pos_px = PF_X + k * 64;
        s->y_pos_px = 34;
    }
    hud_center(13, "THE GAMMA CORE RISES.", HC_GRAY);
    hud_center(14, "ONE SHIP. NO RETREAT.", HC_GRAY);
    title_stage();
    hud_center(22, "Z VULCAN  X HOMING", HC_GRAY);
    hud_center(23, "C SPREAD  SPACE SWORD", HC_GRAY);
    hud_center(24, "Z+X+C HYPER SWORD", HC_GRAY);
    hud_center(25, "COMBINE Z X C FOR MORE", HC_GRAY);
    music_play("ROM:Title.vgm");
}

/* Start the current stage; score, lives and the chain carry over. */
static void begin_stage(void)
{
    gstate = GS_PLAY;
    gtimer = 0;
    paused = false;
    reset_world();
    clear_playfield();
    bg_reset();
    bg_set_speed(stage->speed);
    player_respawn();
    draw_labels();
    stage_reset();
}

static void enter_play(void)
{
    music_stop();
    score_reset();
    player_reset();
    stage_boss_rush = first_stage >= STAGE_COUNT;
    hiscore_select(stage_boss_rush);
    stage_load(stage_boss_rush ? 0 : first_stage);
    begin_stage();
}

static void next_stage(void)
{
    stage_load((uint8_t)(stage_num + 1));
    begin_stage();
}

static void enter_results(void)
{
    gstate = GS_RESULTS;
    gtimer = 0;
    bullets_reset();
    clear_playfield();
    results_bonus = (uint16_t)boss_destruction * 100;
    if (boss_destruction >= 100)
        results_bonus += 10000;
    all_clear = stage_is_last();
    hud_center(4, stage->name, HC_WHITE);
    hud_center(6, boss_escaped ? "BOSS ESCAPED" : "STAGE CLEAR", boss_escaped ? HC_RED : HC_YELLOW);
    hud_text(10, 10, "DESTRUCTION", HC_CYAN, HC_CLEAR);
    hud_u16(23, 10, boss_destruction, 3, HC_WHITE, HC_CLEAR);
    hud_text(26, 10, "%", HC_WHITE, HC_CLEAR);
    hud_text(10, 12, "MAX CHAIN", HC_CYAN, HC_CLEAR);
    hud_u16(23, 12, chain_max, 3, HC_WHITE, HC_CLEAR);
    hud_text(10, 14, "BONUS", HC_CYAN, HC_CLEAR);
    hud_u16(21, 14, results_bonus, 5, HC_YELLOW, HC_CLEAR);
    if (boss_destruction >= 100)
        hud_center(17, "PERFECT DESTRUCTION!", HC_PINK);
    /* Victory music for a destroyed boss; the jingle alone when it got away. */
    if (boss_escaped) {
        music_stop();
        sfx_player(SFX_CLEAR, SFX_PRI_TOP);
    } else {
        music_play_once("ROM:Victory.vgm");
    }
}

static void enter_gameover(void)
{
    hiscore_save();
    gstate = GS_GAMEOVER;
    gtimer = 0;
    hud_center(13, "GAME OVER", HC_RED);
    music_play_once("ROM:Gameover.vgm");
}

static void play_frame(void)
{
    if (in_pressed & IN_START) {
        paused = !paused;
        if (paused)
            hud_center(16, "PAUSE", HC_WHITE);
        else
            hud_clear_row(16);
    }
    if (paused)
        return;

    uint8_t scroll = bg_update();
    tgt_clear();
    stage_update(scroll);
    enemies_update(scroll);
    boss_update();
    player_update(stage_phase != SP_CLEAR);
    shots_update();
    bullets_update();
    fx_update(0);
    score_hud(false);
    player_hud(false);
    boss_hud(false);

    if (pl_state == PL_DEAD && pl_lives == 0) {
        if (++gtimer > 120)
            enter_gameover();
    } else if (stage_phase == SP_CLEAR) {
        if (++gtimer > 90)
            enter_results();
    }
}

int main(void)
{
    xreg_vga_canvas(CANVAS_320X240);
    input_init();
    bg_init();
    spr_init();
    hud_init();
    music_init();
    sfx_init();
    rng_seed(ria_vsync());
    hiscore_load();
    enter_title();

    uint8_t vs = ria_vsync();
    while (1) {
        uint8_t now;
        while ((now = ria_vsync()) == vs)
            ;
        if ((uint8_t)(now - vs) > 1 && !frame_loaded)
            lag_frames++;
        frame_loaded = false;
        vs = now;

        spr_commit();
        bg_commit();
        music_update();
        sfx_update();
        input_update();
        frame_count++;

        switch (gstate) {
        case GS_TITLE:
            bg_update();
            gtimer++;
            hud_center(19, (gtimer & 32) ? "PRESS START" : "           ", HC_YELLOW);
            if (in_pressed & (IN_LEFT | IN_RIGHT)) {
                /* The stages, then the boss rush. */
                first_stage = (uint8_t)((first_stage + (in_pressed & IN_LEFT ? STAGE_COUNT : 1)) % (STAGE_COUNT + 1));
                title_stage();
                sfx_player(SFX_LOCK, SFX_PRI_EVENT);
            } else if (in_pressed & (IN_START | IN_FIRE)) {
                enter_play();
            }
            break;
        case GS_PLAY:
            play_frame();
            break;
        case GS_RESULTS:
            bg_update();
            player_update(false);
            fx_update(0);
            gtimer++;
            /* Count the bonus into the score. */
            if (results_bonus && gtimer > 60) {
                uint16_t step = results_bonus > 500 ? 500 : results_bonus;
                score_add(step);
                results_bonus -= step;
                hud_u16(21, 14, results_bonus, 5, HC_YELLOW, HC_CLEAR);
            }
            score_hud(false);
            if (gtimer > 240 && (in_pressed & (IN_START | IN_FIRE))) {
                hiscore_save();
                if (!all_clear) {
                    next_stage();
                } else if (gtimer < 0x8000) {
                    /* The ending: one more screen, then the title. */
                    gtimer = 0x8000;
                    clear_playfield();
                    hud_center(8, stage_boss_rush ? "BOSS RUSH CLEAR!" : "ALL CLEAR!", HC_YELLOW);
                    hud_center(11, "THE GAMMA CORE IS DEAD.", HC_WHITE);
                    hud_center(13, "THE SUN BURNS CLEAN.", HC_GRAY);
                    hud_center(17, "FINAL SCORE", HC_CYAN);
                    hud_digits(16, 19, score_d, SCORE_DIGITS, HC_YELLOW);
                    sfx_player(SFX_CLEAR, SFX_PRI_TOP);
                } else if (gtimer > 0x8000 + 120) {
                    enter_title();
                }
            }
            break;
        case GS_GAMEOVER: {
            /* The world carries on without the player, so ground targets
             * stay pinned to the scrolling map. Nothing fires at a dead ship. */
            uint8_t scroll = bg_update();
            tgt_clear();
            enemies_update(scroll);
            boss_update();
            shots_update();
            bullets_update();
            fx_update(0);
            gtimer++;
            /* Back to the title when the music ends, or on a button. */
            if (gtimer > 1440 || (gtimer > 90 && (in_pressed & (IN_START | IN_FIRE))))
                enter_title();
            break;
        }
        }
    }
}
