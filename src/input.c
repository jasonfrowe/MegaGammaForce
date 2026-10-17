#include <rp6502.h>
#include "hidkeys.h"
#include "input.h"
#include "xram.h"

uint8_t in_held;
uint8_t in_pressed;

static keyboard_t kb;

typedef struct
{
    uint8_t key;
    uint8_t bits;
} keymap_t;

static const keymap_t keymap[] = {
    {HID_KEY_ARROW_UP, IN_UP},
    {HID_KEY_ARROW_DOWN, IN_DOWN},
    {HID_KEY_ARROW_LEFT, IN_LEFT},
    {HID_KEY_ARROW_RIGHT, IN_RIGHT},
    {HID_KEY_W, IN_UP},
    {HID_KEY_S, IN_DOWN},
    {HID_KEY_A, IN_LEFT},
    {HID_KEY_D, IN_RIGHT},
    {HID_KEY_Z, IN_A},
    {HID_KEY_J, IN_A},
    {HID_KEY_X, IN_B},
    {HID_KEY_K, IN_B},
    {HID_KEY_C, IN_C},
    {HID_KEY_L, IN_C},
    {HID_KEY_V, IN_A | IN_B},
    {HID_KEY_B, IN_B | IN_C},
    {HID_KEY_N, IN_A | IN_C},
    {HID_KEY_SPACE, IN_A | IN_C},
    {HID_KEY_M, IN_A | IN_B | IN_C},
    {HID_KEY_ENTER, IN_START},
    {HID_KEY_P, IN_START},
};

void input_init(void)
{
    xreg_ria_keyboard(XRAM_KEYBOARD);
    xreg_ria_gamepad(XRAM_GAMEPAD);
    in_held = 0;
    in_pressed = 0;
}

void input_update(void)
{
    uint8_t held = 0;

    xram0_read(&kb, XRAM_KEYBOARD, sizeof(kb));
    if (!KEYBOARD_PRESSED(kb.keys, KEYBOARD_NO_KEY)) {
        for (uint8_t i = 0; i < sizeof(keymap) / sizeof(keymap[0]); i++) {
            if (KEYBOARD_PRESSED(kb.keys, keymap[i].key))
                held |= keymap[i].bits;
        }
    }

    uint8_t pad[4];
    xram0_read(pad, XRAM_GAMEPAD, 4);
    if (pad[0] & GAMEPAD_FEAT_CONNECTED) {
        held |= (uint8_t)((pad[0] | pad[1]) & 0x0F); /* d-pad and left stick share bit order */
        uint8_t b0 = pad[2], b1 = pad[3];
        if (b0 & GAMEPAD_BTN0_X)
            held |= IN_A;
        if (b0 & GAMEPAD_BTN0_A)
            held |= IN_B;
        if (b0 & GAMEPAD_BTN0_B)
            held |= IN_C;
        if (b0 & GAMEPAD_BTN0_Y)
            held |= IN_A | IN_C;
        if (b0 & GAMEPAD_BTN0_L1)
            held |= IN_A | IN_B;
        if (b0 & GAMEPAD_BTN0_R1)
            held |= IN_B | IN_C;
        if (b1 & (GAMEPAD_BTN1_L2 | GAMEPAD_BTN1_R2))
            held |= IN_FIRE;
        if (b1 & (GAMEPAD_BTN1_START | GAMEPAD_BTN1_SELECT))
            held |= IN_START;
    }

    in_pressed = (uint8_t)(held & ~in_held);
    in_held = held;
}
