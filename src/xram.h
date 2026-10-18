/* RP6502 XRAM Mapping */

#ifndef XRAM_H
#define XRAM_H

#include <rp6502.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "gen/assets.h"

/* Snippets from the docs, copied unchanged. */
/* https://picocomputer.github.io/sdk.html#xram-memory-map */

/* RIA Keyboard */

#define KEYBOARD_NO_KEY 0
#define KEYBOARD_NUM_LOCK 1
#define KEYBOARD_CAPS_LOCK 2
#define KEYBOARD_SCROLL_LOCK 3

#define KEYBOARD_PRESSED(keys, code) ((keys)[(code) >> 3] & (1 << ((code) & 7)))

#define xreg_ria_keyboard(...) xreg(0, 0, 0, __VA_ARGS__)

typedef struct
{
    uint8_t keys[32];
} keyboard_t; /* layout */

/* RIA Gamepads */

#define GAMEPAD_PLAYERS 4

#define GAMEPAD_DPAD_UP 0x01
#define GAMEPAD_DPAD_DOWN 0x02
#define GAMEPAD_DPAD_LEFT 0x04
#define GAMEPAD_DPAD_RIGHT 0x08

#define GAMEPAD_FEAT_TYPE_MASK 0x30
#define GAMEPAD_FEAT_STICKS 0x40
#define GAMEPAD_FEAT_CONNECTED 0x80

#define GAMEPAD_LSTICK_UP 0x01
#define GAMEPAD_LSTICK_DOWN 0x02
#define GAMEPAD_LSTICK_LEFT 0x04
#define GAMEPAD_LSTICK_RIGHT 0x08

#define GAMEPAD_BTN0_A 0x01
#define GAMEPAD_BTN0_B 0x02
#define GAMEPAD_BTN0_C 0x04
#define GAMEPAD_BTN0_X 0x08
#define GAMEPAD_BTN0_Y 0x10
#define GAMEPAD_BTN0_Z 0x20
#define GAMEPAD_BTN0_L1 0x40
#define GAMEPAD_BTN0_R1 0x80

#define GAMEPAD_BTN1_L2 0x01
#define GAMEPAD_BTN1_R2 0x02
#define GAMEPAD_BTN1_SELECT 0x04
#define GAMEPAD_BTN1_START 0x08
#define GAMEPAD_BTN1_HOME 0x10
#define GAMEPAD_BTN1_L3 0x20
#define GAMEPAD_BTN1_R3 0x40

#define xreg_ria_gamepad(...) xreg(0, 0, 2, __VA_ARGS__)

typedef struct
{
    uint8_t dpad;
    uint8_t sticks;
    uint8_t btn0;
    uint8_t btn1;
    int8_t lx;
    int8_t ly;
    int8_t rx;
    int8_t ry;
    uint8_t l2;
    uint8_t r2;
} gamepad_player_t;

typedef struct
{
    gamepad_player_t player[GAMEPAD_PLAYERS];
} gamepad_t; /* layout */

/* RIA OPL2 */

#define xreg_ria_opl(...) xreg(0, 1, 1, __VA_ARGS__)

typedef struct
{
    uint8_t reg[256];
} opl_t; /* layout */

/* VGA Canvas */

#define xreg_vga_canvas(...) xreg(1, 0, 0, __VA_ARGS__)

#define CANVAS_CONSOLE 0
#define CANVAS_320X240 1
#define CANVAS_320X180 2
#define CANVAS_640X480 3
#define CANVAS_640X360 4

/* VGA Color */

#define COLOR_FROM_RGB8(r, g, b) \
    ((((unsigned)(b) >> 3) << 11) | (((unsigned)(g) >> 3) << 6) | ((unsigned)(r) >> 3))
#define COLOR_FROM_RGB5(r, g, b) \
    (((unsigned)(b) << 11) | ((unsigned)(g) << 6) | (unsigned)(r))
#define COLOR_ALPHA_MASK (1u << 5)

/* VGA Mode 1: Character */

#define xreg_vga_mode1(...) xreg(1, 0, 1, 1, __VA_ARGS__)

#define MODE1_1BPP 0x00
#define MODE1_4BPPR 0x01
#define MODE1_4BPP 0x02
#define MODE1_8BPP 0x03
#define MODE1_16BPP 0x04

#define MODE1_8X8 0x00
#define MODE1_8X16 0x08

#define MODE1_FG_BG(fg, bg) ((uint8_t)(((fg) << 4) | (bg)))
#define MODE1_BG_FG(bg, fg) ((uint8_t)(((bg) << 4) | (fg)))

typedef struct
{
    bool x_wrap;
    bool y_wrap;
    int16_t x_pos_px;
    int16_t y_pos_px;
    int16_t width_chars;
    int16_t height_chars;
    uint16_t xram_data_ptr;
    uint16_t xram_palette_ptr;
    uint16_t xram_font_ptr;
} mode1_config_t; /* layout */

typedef struct
{
    uint8_t glyph_code;
    uint8_t bg_fg_index;
} mode1_4bpp_data_t; /* layout */

/* VGA Mode 2: Tile */

#define xreg_vga_mode2(...) xreg(1, 0, 1, 2, __VA_ARGS__)

#define MODE2_1BPP 0x00
#define MODE2_2BPP 0x01
#define MODE2_4BPP 0x02
#define MODE2_8BPP 0x03

#define MODE2_8X8 0x00
#define MODE2_16X16 0x08

#define MODE2_TILE(bpp, size)                 \
    struct                                    \
    {                                         \
        struct                                \
        {                                     \
            uint8_t cols[(size) * (bpp) / 8]; \
        } rows[size];                         \
    }

typedef struct
{
    bool x_wrap;
    bool y_wrap;
    int16_t x_pos_px;
    int16_t y_pos_px;
    int16_t width_tiles;
    int16_t height_tiles;
    uint16_t xram_data_ptr;
    uint16_t xram_palette_ptr;
    uint16_t xram_tile_ptr;
} mode2_config_t; /* layout */

/* VGA Mode 5: Sprite */

#define xreg_vga_mode5(...) xreg(1, 0, 1, 5, __VA_ARGS__)

#define MODE5_1BPP 0x00
#define MODE5_2BPP 0x01
#define MODE5_4BPP 0x02
#define MODE5_8BPP 0x03

#define MODE5_CUSTOM 0x38

#define MODE5_HFLIP 0x10
#define MODE5_VFLIP 0x20
#define MODE5_HDOUBLE 0x40
#define MODE5_VDOUBLE 0x80

#define MODE5_SIZE(width, height) \
    ((((height) / 4 - 1) << 4) | ((width) / 4 - 1))

typedef struct
{
    int16_t x_pos_px;
    int16_t y_pos_px;
    uint16_t xram_sprite_ptr;
    uint16_t palette_ptr;
    uint8_t width_height;
    uint8_t options;
} mode5_csprite_t; /* layout */

/* ------------------------------------------------------------------ */
/* MegaGammaForce's XRAM.                                             */
/* ------------------------------------------------------------------ */

/* Plane 1 sprite list, back to front. */
#define SPR_BOSS 0
#define SPR_BOSS_N 16
#define SPR_ENEMY (SPR_BOSS + SPR_BOSS_N)
#define SPR_ENEMY_N 24
#define SPR_SHOT (SPR_ENEMY + SPR_ENEMY_N)
#define SPR_SHOT_N 32
#define SPR_PLAYER (SPR_SHOT + SPR_SHOT_N)
#define SPR_FLAME (SPR_PLAYER + 1)
#define SPR_SWORD (SPR_FLAME + 2)
#define SPR_BEAM (SPR_SWORD + 1)
#define SPR_BEAM_N 3
#define SPR_LOCK (SPR_BEAM + SPR_BEAM_N) /* lock-on ring dots, then reticles */
#define SPR_LOCK_RING 8
#define SPR_LOCK_N 14
#define SPR_FX (SPR_LOCK + SPR_LOCK_N)
#define SPR_FX_N 14
#define SPR_BULLET (SPR_FX + SPR_FX_N)
#define SPR_BULLET_N 64
#define SPR_COUNT (SPR_BULLET + SPR_BULLET_N)

/* Plane 0 sprite list: ground units, under everything airborne. Each stage
 * shows as many of them as it needs (stage_def_t.gspr_n), up to this. */
#define GSPR_MAX 16

/* Plane 2 sprite list: HUD graphics, in front of the text panels. */
#define HSPR_ICON 0      /* the weapon's icon */
#define HSPR_LIFE 1      /* the ship beside the lives count */
#define HSPR_GEM 2       /* three chain gems */
#define HSPR_SCORE 5     /* seven score digits */
#define HSPR_HI 12       /* seven high score digits */
#define HSPR_STAGE 19    /* the stage number */
#define HSPR_HYPER 20    /* the Hyper Sword's ready sign */
#define HSPR_PORTRAIT 21 /* the boss */
#define HSPR_BANNER 22   /* "STAGE" and its number, or "WARNING", doubled */
#define HSPR_N 24

/* HUD text grid, 8x8 font on the 320x240 canvas. */
#define HUD_COLS 40
#define HUD_ROWS 30

#define SFX_DATA_MAX 4096

typedef struct
{
    uint16_t c[16];
} palette_t;

typedef struct
{
    /* First, so the OPL2 registers start on a page boundary. */
    opl_t opl;

    keyboard_t keyboard;
    gamepad_t gamepad;
    mode2_config_t bg_config;    /* plane 0 fill: the ground */
    mode2_config_t layer_config; /* plane 1 fill: parallax sky, or a boss */
    mode1_config_t hud_config;
    mode5_csprite_t spr[SPR_COUNT];
    mode5_csprite_t gspr[GSPR_MAX];
    mode5_csprite_t hspr[HSPR_N];
    mode1_4bpp_data_t hud[HUD_ROWS][HUD_COLS];

    /* Loaded from the ROM by CMakeLists.txt. The last PAL_STAGE_N
     * palettes are replaced by each stage's own. */
    palette_t palettes[PAL_COUNT];
    uint8_t img_player[PLAYER_FRAMES][PLAYER_FRAME_BYTES];
    uint8_t img_flame[FLAME_FRAMES][FLAME_FRAME_BYTES];
    uint8_t img_vulcan[VULCAN_FRAMES][VULCAN_FRAME_BYTES];
    uint8_t img_shot8[SHOT8_FRAMES][SHOT8_FRAME_BYTES];
    uint8_t img_sword[SWORD_FRAMES][SWORD_FRAME_BYTES];
    uint8_t img_beam[BEAM_FRAMES][BEAM_FRAME_BYTES];
    uint8_t img_missile[MISSILE_FRAMES][MISSILE_FRAME_BYTES];
    uint8_t img_lock[LOCK_FRAMES][LOCK_FRAME_BYTES];
    uint8_t img_enemy16[ENEMY16_FRAMES][ENEMY16_FRAME_BYTES];
    uint8_t img_enemy32[ENEMY32_FRAMES][ENEMY32_FRAME_BYTES];
    uint8_t img_bullet8[BULLET8_FRAMES][BULLET8_FRAME_BYTES];
    uint8_t img_bullet16[BULLET16_FRAMES][BULLET16_FRAME_BYTES];
    uint8_t img_expl16[EXPL16_FRAMES][EXPL16_FRAME_BYTES];
    uint8_t img_expl32[EXPL32_FRAMES][EXPL32_FRAME_BYTES];
    uint8_t img_miniship[MINISHIP_FRAMES][MINISHIP_FRAME_BYTES];
    uint8_t img_gem[GEM_FRAMES][GEM_FRAME_BYTES];
    uint8_t img_icons[ICONS_FRAMES][ICONS_FRAME_BYTES];
    uint8_t img_digits[DIGITS_FRAMES][DIGITS_FRAME_BYTES];
    uint8_t img_banner[BANNER_FRAMES][BANNER_FRAME_BYTES];
    uint8_t img_bannernum[BANNERNUM_FRAMES][BANNERNUM_FRAME_BYTES];
    uint8_t img_warn_a[WARN_A_FRAMES][WARN_A_FRAME_BYTES];
    uint8_t img_warn_b[WARN_B_FRAMES][WARN_B_FRAME_BYTES];
    uint8_t sfx_data[SFX_DATA_MAX];
    /* XRAM_BANK, the rest of XRAM, follows: the current stage's script,
     * ground and sky, then XRAM_BOSS, its boss, each loaded from the ROM
     * (see stage.c). */
} xram_layout_t;

#define XRAM_OPL offsetof(xram_layout_t, opl)
_Static_assert((XRAM_OPL & 0xFF) == 0, "The OPL2 registers must start on a page boundary.");

#define XRAM_KEYBOARD offsetof(xram_layout_t, keyboard)
#define XRAM_GAMEPAD offsetof(xram_layout_t, gamepad)
#define XRAM_BG_CONFIG offsetof(xram_layout_t, bg_config)
#define XRAM_LAYER_CONFIG offsetof(xram_layout_t, layer_config)
#define XRAM_HUD_CONFIG offsetof(xram_layout_t, hud_config)
#define XRAM_SPR offsetof(xram_layout_t, spr)
#define XRAM_GSPR offsetof(xram_layout_t, gspr)
#define XRAM_HSPR offsetof(xram_layout_t, hspr)
#define XRAM_HUD offsetof(xram_layout_t, hud)
#define XRAM_PALETTES offsetof(xram_layout_t, palettes)
#define XRAM_IMG_PLAYER offsetof(xram_layout_t, img_player)
#define XRAM_IMG_FLAME offsetof(xram_layout_t, img_flame)
#define XRAM_IMG_VULCAN offsetof(xram_layout_t, img_vulcan)
#define XRAM_IMG_SHOT8 offsetof(xram_layout_t, img_shot8)
#define XRAM_IMG_SWORD offsetof(xram_layout_t, img_sword)
#define XRAM_IMG_BEAM offsetof(xram_layout_t, img_beam)
#define XRAM_IMG_MISSILE offsetof(xram_layout_t, img_missile)
#define XRAM_IMG_LOCK offsetof(xram_layout_t, img_lock)
#define XRAM_IMG_ENEMY16 offsetof(xram_layout_t, img_enemy16)
#define XRAM_IMG_ENEMY32 offsetof(xram_layout_t, img_enemy32)
#define XRAM_IMG_BULLET8 offsetof(xram_layout_t, img_bullet8)
#define XRAM_IMG_BULLET16 offsetof(xram_layout_t, img_bullet16)
#define XRAM_IMG_EXPL16 offsetof(xram_layout_t, img_expl16)
#define XRAM_IMG_EXPL32 offsetof(xram_layout_t, img_expl32)
#define XRAM_IMG_MINISHIP offsetof(xram_layout_t, img_miniship)
#define XRAM_IMG_GEM offsetof(xram_layout_t, img_gem)
#define XRAM_IMG_ICONS offsetof(xram_layout_t, img_icons)
#define XRAM_IMG_DIGITS offsetof(xram_layout_t, img_digits)
#define XRAM_IMG_BANNER offsetof(xram_layout_t, img_banner)
#define XRAM_IMG_BANNERNUM offsetof(xram_layout_t, img_bannernum)
#define XRAM_IMG_WARN_A offsetof(xram_layout_t, img_warn_a)
#define XRAM_IMG_WARN_B offsetof(xram_layout_t, img_warn_b)
#define XRAM_SFX_DATA offsetof(xram_layout_t, sfx_data)
#define XRAM_BANK sizeof(xram_layout_t)
#define XRAM_BOSS (XRAM_BANK + STAGE_BANK_MAX)
/* The top page of XRAM is left free. */
#define XRAM_BANK_SIZE (0xFF00 - XRAM_BANK)

#define PAL_ADDR(p) (XRAM_PALETTES + (p) * sizeof(palette_t))

/* The FPGA VGA caches 1 KB of palettes: keep them all together, within it. */
_Static_assert(sizeof(palette_t) * PAL_COUNT / sizeof(uint16_t) <= 512,
               "palettes hold more than 512 colors.");
_Static_assert((uint32_t)STAGE_BANK_MAX + BOSS_BANK_MAX <= XRAM_BANK_SIZE, "a stage and its boss do not fit in XRAM.");

#endif
