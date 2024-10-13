/*
 * SetDur.c
 *
 *  Created on: Oct 13, 2024
 *      Author: hadha
 */
#include "SetDur.h"


#include "SettingsStore.h"

void setDur(unsigned char id, unsigned char* dec) {
    setSetting(REPETITIONS, (*(dec+1))>>1);
    setSetting(MODE_OF_OPERATION, (*(dec+1))%2);

    int dur = *(dec+2);
    for(__uint8_t i = 0; i <= 8; i++) {
        dur *= 2;
    }
    dur += *(dec+3);
    setSetting(DURATION, dur);

    int breaktime = *(dec+4);
    for(__uint8_t j = 0; j <= 8; j++) {
        breaktime *= 2;
    }
    breaktime += *(dec+5);
    setSetting(BREAKTIME, dur);
}

