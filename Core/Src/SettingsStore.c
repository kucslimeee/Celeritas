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
#include "Flash.h"


typedef struct {
    SettingType type;
    uint16_t  value;
}Setting;

#define SETTING_COUNT 9

Setting settings[SETTING_COUNT] = {
	{MODE_OF_OPERATION, MAX_HITS},
	{IS_OKAY, 0},
	{DURATION, 10},
	{BREAKTIME, 0},
	{REPETITIONS, 1},
	{MIN_VOLTAGE, 200},
	{MAX_VOLTAGE, 4095},
	{RESOLUTION, 16},
	{SAMPLES, 3}
};

void settingStoreInit() {
	uint16_t load_length = SETTING_COUNT * 4;
	uint16_t loaded_data[load_length];
	flash_load(SETTINGS_ADDR, load_length, &loaded_data);
	if(loaded_data[0] != 0x00 || loaded_data[1] != 0x00) return;
	for (uint16_t i = 0; i < SETTING_COUNT; i++) {
		SettingType type = loaded_data[i*4 + 1] << 8 | loaded_data[i*4];
		settings[type].value = loaded_data[i*4 + 3] << 8 | loaded_data[i*4+2];
	}
}

uint16_t getSetting(SettingType setting) {
    return settings[setting].value;
}

void setSetting(SettingType setting, uint16_t value) {
    settings[setting].value = value;
}

void formatSetting(Setting* setting, uint16_t offset, uint16_t* output);
void saveSettings();

void formatSetting(Setting* setting, uint16_t offset, uint16_t* output) {
	output[offset + 0] = setting->type & 0xFF;
	output[offset + 1] = (setting->type >> 8) & 0xFF;
	output[offset + 2] = setting->value & 0xFF;
	output[offset + 3] = (setting->value >> 8) & 0xFF;
}

void saveSettings() {
	uint16_t save_length = SETTING_COUNT * 4;
	uint16_t save_data [save_length];
	for(int i = 0; i < SETTING_COUNT; i++) {
		formatSetting(&settings[i], i*4, &save_data);
	}
	flash_save(SETTINGS_ADDR, save_length, &save_data);
}
