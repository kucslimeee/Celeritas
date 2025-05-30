//
// Created by andras on 2024.10.04..
//

#ifndef SETTINGSTORE_H
#define SETTINGSTORE_H
#include<stdint.h>

typedef enum  {
    MODE_OF_OPERATION = 0,
	IS_OKAY,
	DURATION,
	BREAKTIME,
	REPETITIONS,
    MIN_VOLTAGE,
	MAX_VOLTAGE,
	RESOLUTION,
	SAMPLES
}SettingType;


void settingStoreInit();
uint16_t getSetting(SettingType setting);

void setSetting(SettingType setting, uint16_t value);
void saveSettings();
void clear_saved_Settings();

#endif //SETTINGSTORE_H
