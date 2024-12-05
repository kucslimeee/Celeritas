/*
 * Measurements.c
 *
 *  Created on: Aug 9, 2024
 *      Author: hadha
 */

#include "Measurements.h"
#include "Channels.h"
#include "main.h"
#include "exp_i2c_slave.h"
#include "i2c_queue.h"
#include "Scheduler.h"
#include <stdbool.h>
#include <stdlib.h>

extern ADC_HandleTypeDef hadc1;
extern volatile RunningState status;

uint16_t sample_adc(uint8_t samples, uint16_t min_voltage, uint16_t max_voltage, bool is_okay);


void measure(Request request){
	uint8_t resolution = request.resolution;
	uint8_t measurementData[resolution];
	for(int i = 0; i < resolution; i++) measurementData[i] = 0x00;
	uint8_t intervalLength = (request.max_voltage - request.min_voltage)/resolution;
	uint8_t intervalSize = 4080 / resolution; // the maximum number of peaks that a category can store. 4080 = 255 * 16
	uint8_t peaks = 0;

	select_measurement_channel();
	HAL_ADC_Start(&hadc1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 1);

	HAL_Delay(1000);
	while(peaks < request.limit){
		uint16_t sample = sample_adc(request.samples, request.min_voltage, request.max_voltage, request.is_okay);
		if(!sample) break;
		uint8_t intervalIndex = abs(sample - request.min_voltage)/intervalLength;
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, peaks % 2);
		if(measurementData[intervalIndex] < intervalSize) measurementData[intervalIndex]++;
		if(request.type == MAX_HITS) peaks++;
	}

	HAL_ADC_Stop(&hadc1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 0);
	if(request.is_header){
		if(request.is_priority){
			add_spectrum(request, &measurementData, resolution);
			add_header(request, request.limit);
		} else {
			add_header(request, request.limit);
			add_spectrum(request, &measurementData, resolution);
			}
	}else {
		add_spectrum(request, &measurementData, resolution);
	}
	scheduler_finish_measurement();
}

/*
 * BEFORE YOU CALL make SURE ADC1 is INITIALIZED AND the selected channel is ADC_IN_0 !!!!
 */
uint16_t sample_adc(uint8_t samples, uint16_t min_voltage, uint16_t max_voltage, bool is_okay){
	uint16_t sum = 0;

	while(1){
		if(status != RUNNING) {
			return 0;
		}
		sum = analogRead(); //measure ADC
		if(!(sum > min_voltage && sum < max_voltage)) continue;
		for(int i = 1; i < samples; i++){
			sum += analogRead();
		}
		break;
	}
	if(is_okay) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	uint16_t voltage;
	do {
		voltage = analogRead();
	}while (voltage > min_voltage);
	if(is_okay) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	return (uint16_t)(sum/samples);
}

uint16_t analogRead()
{
	HAL_ADC_PollForConversion(&hadc1, 100); // poll for conversion
	return HAL_ADC_GetValue(&hadc1); // get the adc value
}

uint16_t get_temperature() {
	select_temperature_channel();
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	uint16_t adc = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return adc;
}
