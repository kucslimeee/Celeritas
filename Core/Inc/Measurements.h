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
int8_t get_temperature(uint16_t input_refint_voltage);
uint16_t get_refint_voltage();


#endif /* SRC_MEASUREMENTS_H_ */
