#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

/*
 * Controls
 *
 *            Keyboard           Gamepad
 * Move       arrows / WASD      d-pad / left stick
 * A Vulcan   Z or J             X
 * B Homing   X or K             A
 * C Spread   C or L             B
 * A+B        V                  L1          back wide
 * B+C        B                  R1          homing plasma
 * A+C        N or Space         Y           radiant sword
 * A+B+C      M                  L2 / R2     hyper sword (when charged)
 * Start      Enter / P          Start
 */

#define IN_UP 0x01
#define IN_DOWN 0x02
#define IN_LEFT 0x04
#define IN_RIGHT 0x08
#define IN_A 0x10
#define IN_B 0x20
#define IN_C 0x40
#define IN_START 0x80

#define IN_FIRE (IN_A | IN_B | IN_C)

extern uint8_t in_held;
extern uint8_t in_pressed; /* went down this frame */

void input_init(void);
void input_update(void);

#endif
