/*
 * SetScale.c
 *
 *  Created on: Oct 13, 2024
 *      Author: hadha
 */
#include "Commands/SetScale.h"
#include "SettingsStore.h"
#include "i2c_queue.h"
#include <stdbool.h>

bool check_res (uint16_t num) {
	const uint16_t items[] = {0, 1, 2, 4, 8, 16, 32, 64, 128};	//the packet sizes, but see Request.h
    for (int i = 0; i < 9; i++)
    {
    	if (num == items[i]) return true;
    }
    return false;
}


void setScale(uint8_t id, uint8_t* dec) {
	uint16_t resolution = (uint16_t)dec[3];
	if (!check_res(resolution)) {
		add_error(id, CORRUPTED);
	    return;
	}
    setSetting(MIN_VOLTAGE, (uint16_t)((dec[0] << 4) + (dec[1] >> 4)));
    setSetting(MAX_VOLTAGE, (uint16_t)(((dec[1]%16)<<8)+dec[2]));

    if(resolution == 0){setSetting(RESOLUTION, 1);} else{setSetting(RESOLUTION, resolution * 8);};
    setSetting(SAMPLES, (uint16_t)dec[4]);
}
