#ifndef STAGE_H
#define STAGE_H

#include <stdbool.h>
#include <stdint.h>

enum
{
    SP_INTRO,
    SP_WAVES,
    SP_WARNING,
    SP_BOSS,
    SP_CLEAR,
};

extern uint8_t stage_phase;

void stage_reset(void);
/* Runs the script for this frame; scroll is the pixels scrolled. */
void stage_update(uint8_t scroll);
void stage_chain_popup(uint16_t bonus);
/* Big centered message in the playfield for a time; NULL clears it. */
void stage_message(const char *line1, const char *line2, uint8_t frames);

#endif
