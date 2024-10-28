/*
 * SetScale.c
 *
 *  Created on: Oct 13, 2024
 *      Author: hadha
 */
#include "Commands/SetScale.h"
#include "SettingsStore.h"


void setScale(uint8_t id, uint8_t* dec) {
    setSetting(MIN_VOLTAGE, (uint16_t)((dec[0] << 4) + (dec[1] >> 4)));
    setSetting(MAX_VOLTAGE, (uint16_t)(((dec[1]%16)<<8)+dec[2]));
    setSetting(RESOLUTION, (uint16_t)dec[3]);
    setSetting(SAMPLES, (uint16_t)dec[4]);
}
