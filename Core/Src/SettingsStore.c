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


typedef struct {
    enum SettingType;
    __uint8_t  value;
}Setting;

Setting settings[8] = {0};

__uint8_t getSetting(SettingType setting) {
    return settings[setting];
}

void setSetting(SettingType setting, __uint8_t value) {
    settings[setting] = value;
}

