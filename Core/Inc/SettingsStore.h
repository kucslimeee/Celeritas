//
// Created by andras on 2024.10.04..
//

#ifndef SETTINGSTORE_H
#define SETTINGSTORE_H
#include<ctype.h>
#endif //SETTINGSTORE_H
typedef enum  {
    MODE_OF_OPERATION = 0, DURATION, BREAKTIME, REPETITIONS,
    MIN_VOLTAGE, MAX_VOLTAGE, RESOLUTION, SAMPLES
}SettingType;

__uint8_t getSetting(SettingType setting);

void setSetting(SettingType setting, __uint8_t value);

