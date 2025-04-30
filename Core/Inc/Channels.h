/*
 * Channels.h
 *
 *  Created on: Nov 30, 2024
 *      Author: hpraszpi
 */

#include <stdint.h>

#ifndef INC_CHANNELS_H_
#define INC_CHANNELS_H_

#define ADC_CHANNEL_MEASUREMENT ADC_CHANNEL_1

void select_channel(uint32_t channel, uint32_t sampling_time);

void select_temperature_channel();
void select_measurement_channel();
void select_refint_channel();
void select_vbat_channel();



#endif /* INC_CHANNELS_H_ */
