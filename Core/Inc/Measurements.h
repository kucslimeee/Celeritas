/*
 * Measurements.h
 *
 *  Created on: Aug 9, 2024
 *      Author: hadha
 */

#ifndef SRC_MEASUREMENTS_H_
#define SRC_MEASUREMENTS_H_
#include"Request.h"



void measure(Request request);
uint16_t sample_adc(uint8_t samples, uint16_t min_voltage, uint16_t max_voltage, bool is_okay);
void select_measure_adc();


#endif /* SRC_MEASUREMENTS_H_ */
