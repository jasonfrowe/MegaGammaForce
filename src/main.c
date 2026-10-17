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

uint8_t frame_count;

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

static void draw_labels(void)
{
    hud_frame();
    hud_text(0, 2, "SCORE", HC_CYAN, HC_PANEL);
    hud_text(0, 5, "HISCORE", HC_CYAN, HC_PANEL);
    hud_text(0, 9, "CHAIN", HC_CYAN, HC_PANEL);
    hud_text(0, 12, "BONUS", HC_GRAY, HC_PANEL);
    hud_text(0, 26, "STAGE", HC_GRAY, HC_PANEL);
    hud_text(0, 27, " 1", HC_WHITE, HC_PANEL);
    hud_text(33, 2, "WEAPON", HC_CYAN, HC_PANEL);
    hud_text(33, 6, "SWORD", HC_CYAN, HC_PANEL);
    hud_text(33, 10, "LIVES", HC_CYAN, HC_PANEL);
    score_hud(true);
    player_hud(true);
}

static void enter_title(void)
{
    gstate = GS_TITLE;
    gtimer = 0;
    reset_world();
    bg_reset();
    bg_set_speed(96);
    hud_frame();
    clear_playfield();
    for (uint8_t k = 0; k < 3; k++) {
        mode5_csprite_t *s = &spr[SPR_BOSS + k];
        spr_setup(s, XRAM_IMG_LOGO + k * LOGO_FRAME_BYTES, PAL_ADDR(PAL_BOSS), WH(64, 64), OPT4);
        s->x_pos_px = PF_X + k * 64;
        s->y_pos_px = 34;
    }
    hud_center(13, "THE GAMMA CORE RISES.", HC_GRAY);
    hud_center(14, "ONE SHIP. NO RETREAT.", HC_GRAY);
    hud_center(20, "Z VULCAN  X HOMING", HC_GRAY);
    hud_center(21, "C SPREAD  N SWORD", HC_GRAY);
    hud_center(22, "M HYPER SWORD", HC_GRAY);
    hud_center(23, "COMBINE Z X C FOR MORE", HC_GRAY);
    hud_text(0, 5, "HISCORE", HC_CYAN, HC_PANEL);
    hud_digits(0, 6, hiscore_d, SCORE_DIGITS, HC_YELLOW);
    music_play("ROM:Title.vgm");
}

static void enter_play(void)
{
    gstate = GS_PLAY;
    gtimer = 0;
    paused = false;
    reset_world();
    clear_playfield();
    bg_reset();
    bg_set_speed(160);
    score_reset();
    player_reset();
    draw_labels();
    stage_reset();
}

static void enter_results(void)
{
    gstate = GS_RESULTS;
    gtimer = 0;
    bullets_reset();
    clear_playfield();
    results_bonus = (uint16_t)boss_destruction * 300;
    if (boss_destruction >= 100)
        results_bonus += 20000;
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
    music_stop();
    sfx_player(SFX_CLEAR, SFX_PRI_TOP);
}

static void enter_gameover(void)
{
    hiscore_save();
    gstate = GS_GAMEOVER;
    gtimer = 0;
    hud_center(13, "GAME OVER", HC_RED);
    music_stop();
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
        if ((uint8_t)(now - vs) > 1)
            lag_frames++;
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
            hud_center(17, (gtimer & 32) ? "PRESS START" : "           ", HC_WHITE);
            if (in_pressed & (IN_START | IN_FIRE))
                enter_play();
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
                enter_title();
            }
            break;
        case GS_GAMEOVER:
            bg_update();
            fx_update(0);
            bullets_update();
            gtimer++;
            if (gtimer > 300 || (gtimer > 90 && (in_pressed & (IN_START | IN_FIRE))))
                enter_title();
            break;
        }
    }
}
