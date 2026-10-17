#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

/*
 * Controls. A, B and C are the three guns; holding two of them combines
 * them. Every combination also has a button of its own.
 *
 *                   Keyboard        Gamepad
 * Move              WASD / arrows   d-pad / left stick
 * A   Vulcan        Z or J          A
 * B   Homing        X or K          B
 * C   Spread        C or L          C or R1
 * A+B Homing Plasma V               X
 * A+C Wide Shot     (Z+C)           Y
 * B+C Homing Spread B               Z or L1
 * Radiant Sword     N or Space      R2
 * Hyper Sword       M               L2
 * Start / pause     Enter / P       Start
 */

#define IN_UP 0x0001
#define IN_DOWN 0x0002
#define IN_LEFT 0x0004
#define IN_RIGHT 0x0008
#define IN_A 0x0010
#define IN_B 0x0020
#define IN_C 0x0040
#define IN_START 0x0080
#define IN_SWORD 0x0100
#define IN_HYPER 0x0200

#define IN_GUNS (IN_A | IN_B | IN_C)
#define IN_FIRE (IN_GUNS | IN_SWORD | IN_HYPER)

extern uint16_t in_held;
extern uint16_t in_pressed; /* went down this frame */

void input_init(void);
void input_update(void);

#endif
