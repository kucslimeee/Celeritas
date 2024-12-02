/*
 * Channels.c
 *
 *  Created on: Nov 30, 2024
 *      Author: hpraszpi
 *
 *
 *  Manages ADC channels.
 */
#include "Channels.h"
#include "main.h"
#include <stdint.h>

extern ADC_HandleTypeDef hadc1;
void select_channel(uint32_t channel);

void select_channel(uint32_t channel) {
	ADC_ChannelConfTypeDef sConfig;
	sConfig.Channel = channel;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

void select_temperature_channel(){
	select_channel(ADC_CHANNEL_TEMPSENSOR);
}

void select_measurement_channel() {
	select_channel(ADC_CHANNEL_MEASUREMENT);
}
