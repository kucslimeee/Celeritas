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
uint16_t analogRead();
uint16_t get_temperature();
void select_measure_adc();


#endif /* SRC_MEASUREMENTS_H_ */
