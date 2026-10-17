#ifndef HUD_H
#define HUD_H

#include <stdint.h>

/* HUD palette (palettes.HUD) */
enum
{
    HC_CLEAR = 0,
    HC_PANEL = 1,
    HC_BORDER = 2,
    HC_WHITE = 3,
    HC_GRAY = 4,
    HC_CYAN = 5,
    HC_YELLOW = 6,
    HC_RED = 7,
    HC_BLUE = 8,
    HC_GREEN = 9,
    HC_ORANGE = 10,
    HC_PINK = 11,
    HC_DIM = 12,
    HC_DARK = 13,
    HC_VIOLET = 14,
    HC_BLACK = 15,
};

#define HUD_LEFT 0
#define HUD_RIGHT 32
#define HUD_PANEL_W 8
#define HUD_PF_COL 8 /* first column over the playfield */

void hud_init(void);
/* Draw both panels empty and clear the playfield text. */
void hud_frame(void);
void hud_text(uint8_t col, uint8_t row, const char *s, uint8_t fg, uint8_t bg);
/* Text centered in the playfield, transparent background. */
void hud_center(uint8_t row, const char *s, uint8_t fg);
void hud_clear_row(uint8_t row);
void hud_fill(uint8_t col, uint8_t row, uint8_t n, uint8_t glyph, uint8_t fg, uint8_t bg);
void hud_digits(uint8_t col, uint8_t row, const uint8_t *digits, uint8_t n, uint8_t fg);
void hud_u16(uint8_t col, uint8_t row, uint16_t v, uint8_t width, uint8_t fg, uint8_t bg);
void hud_bar(uint8_t col, uint8_t row, uint8_t width, uint8_t filled_eighths_total, uint8_t fg);

#endif
