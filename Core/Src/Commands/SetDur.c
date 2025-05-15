/*
 * SetDur.c
 *
 *  Created on: Oct 13, 2024
 *      Author: hadha
 */
#include "Commands/SetDur.h"
#include "Request.h"
#include "SettingsStore.h"
#include "main.h"

void setDur(uint8_t id, uint8_t* dec) {
    setSetting(REPETITIONS, (*dec)>>2);
    setSetting(MODE_OF_OPERATION, (((*dec)>>1)%2) ? MAX_HITS : MAX_TIME);
    setSetting(IS_OKAY, (*dec % 2));
    int dur = *(dec+1) << 8;
    dur += *(dec+2);
    setSetting(DURATION, dur);

    int breaktime = *(dec+3);
    for(__uint8_t j = 0; j <= 8; j++) {
        breaktime *= 2;
    }
    breaktime += *(dec+4);
    setSetting(BREAKTIME, breaktime);
}

