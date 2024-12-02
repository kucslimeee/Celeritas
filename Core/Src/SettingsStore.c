/*
 * SettingsStore.c
 *
 *  Created on: Oct 13, 2024
 *      Author: hadha
 */


//
// Created by andras on 2024.10.04..
//

#include "SettingsStore.h"
#include "Request.h"


typedef struct {
    SettingType type;
    uint16_t  value;
}Setting;

Setting settings[9] = {
	{MODE_OF_OPERATION, MAX_HITS},
	{IS_OKAY, 0},
	{DURATION, 10},
	{BREAKTIME, 0},
	{REPETITIONS, 0},
	{MIN_VOLTAGE, 700},
	{MAX_VOLTAGE, 4095},
	{RESOLUTION, 16},
	{SAMPLES, 3}
};

uint16_t getSetting(SettingType setting) {
    return settings[setting].value;
}

void setSetting(SettingType setting, uint16_t value) {
    settings[setting].value = value;
}

