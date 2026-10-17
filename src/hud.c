#include <rp6502.h>
#include <string.h>
#include "hud.h"
#include "xram.h"

/*
 * Mode 1 text on plane 2, in front of everything. The two side panels are
 * opaque, so they also hide sprites as they leave the playfield. Cells over
 * the playfield have background color 0, which is transparent.
 */

#define CELL(col, row) (XRAM_HUD + ((uint16_t)(row) * HUD_COLS + (col)) * 2)

void hud_init(void)
{
    mode1_config_t cfg = {
        .x_wrap = false,
        .y_wrap = false,
        .x_pos_px = 0,
        .y_pos_px = 0,
        .width_chars = HUD_COLS,
        .height_chars = HUD_ROWS,
        .xram_data_ptr = XRAM_HUD,
        .xram_palette_ptr = PAL_ADDR(PAL_HUD),
        .xram_font_ptr = 0xFFFF,
    };
    xram0_write(XRAM_HUD_CONFIG, &cfg, sizeof(cfg));
    hud_frame();
    xreg_vga_mode1(MODE1_4BPP | MODE1_8X8, XRAM_HUD_CONFIG, 2, 0, 0);
}

void hud_frame(void)
{
    uint8_t row[HUD_COLS * 2];
    for (uint8_t c = 0; c < HUD_COLS; c++) {
        bool panel = c < HUD_PANEL_W || c >= HUD_RIGHT;
        row[c * 2] = ' ';
        row[c * 2 + 1] = MODE1_BG_FG(panel ? HC_PANEL : HC_CLEAR, HC_WHITE);
    }
    /* Inner border of each panel, CP437 double vertical line. */
    row[(HUD_PANEL_W - 1) * 2] = 0xBA;
    row[(HUD_PANEL_W - 1) * 2 + 1] = MODE1_BG_FG(HC_PANEL, HC_BORDER);
    row[HUD_RIGHT * 2] = 0xBA;
    row[HUD_RIGHT * 2 + 1] = MODE1_BG_FG(HC_PANEL, HC_BORDER);
    for (uint8_t r = 0; r < HUD_ROWS; r++) {
        xram0_write(CELL(0, r), row, sizeof(row));
    }
}

void hud_text(uint8_t col, uint8_t row, const char *s, uint8_t fg, uint8_t bg)
{
    uint8_t attr = MODE1_BG_FG(bg, fg);
    RIA.addr0 = CELL(col, row);
    RIA.step0 = 1;
    while (*s) {
        RIA.rw0 = (uint8_t)*s++;
        RIA.rw0 = attr;
    }
}

void hud_fill(uint8_t col, uint8_t row, uint8_t n, uint8_t glyph, uint8_t fg, uint8_t bg)
{
    uint8_t attr = MODE1_BG_FG(bg, fg);
    RIA.addr0 = CELL(col, row);
    RIA.step0 = 1;
    while (n--) {
        RIA.rw0 = glyph;
        RIA.rw0 = attr;
    }
}

void hud_center(uint8_t row, const char *s, uint8_t fg)
{
    uint8_t n = (uint8_t)strlen(s);
    hud_text((uint8_t)(HUD_PF_COL + (24 - n) / 2), row, s, fg, HC_CLEAR);
}

void hud_clear_row(uint8_t row)
{
    hud_fill(HUD_PF_COL, row, 24, ' ', HC_WHITE, HC_CLEAR);
}

void hud_digits(uint8_t col, uint8_t row, const uint8_t *digits, uint8_t n, uint8_t fg)
{
    uint8_t attr = MODE1_BG_FG(HC_PANEL, fg);
    RIA.addr0 = CELL(col, row);
    RIA.step0 = 1;
    for (uint8_t i = 0; i < n; i++) {
        RIA.rw0 = (uint8_t)('0' + digits[i]);
        RIA.rw0 = attr;
    }
}

void hud_u16(uint8_t col, uint8_t row, uint16_t v, uint8_t width, uint8_t fg, uint8_t bg)
{
    char buf[6];
    uint8_t i = 5;
    buf[5] = 0;
    do {
        buf[--i] = (char)('0' + v % 10);
        v /= 10;
    } while (v && i);
    while (i > 5 - width)
        buf[--i] = ' ';
    hud_text(col, row, buf + i, fg, bg);
}

void hud_bar(uint8_t col, uint8_t row, uint8_t width, uint8_t filled, uint8_t fg)
{
    /* Each cell shows full (0xDB), half (0xDD) or empty (0xFA). filled is in half cells. */
    uint8_t attr = MODE1_BG_FG(HC_PANEL, fg);
    uint8_t dim = MODE1_BG_FG(HC_PANEL, HC_DIM);
    RIA.addr0 = CELL(col, row);
    RIA.step0 = 1;
    for (uint8_t i = 0; i < width; i++) {
        uint8_t g;
        if (filled >= 2) {
            g = 0xDB;
            filled -= 2;
            RIA.rw0 = g;
            RIA.rw0 = attr;
        } else if (filled == 1) {
            filled = 0;
            RIA.rw0 = 0xDD;
            RIA.rw0 = attr;
        } else {
            RIA.rw0 = 0xFA;
            RIA.rw0 = dim;
        }
    }
}
