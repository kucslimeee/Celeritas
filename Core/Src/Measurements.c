/*
 * Measurements.c
 *
 *  Created on: Aug 9, 2024
 *      Author: hadha
 */

#include "Measurements.h"
#include "main.h"
#include "exp_i2c_slave.h"
#include "i2c_queue.h"
#include "Scheduler.h"
#include <stdbool.h>

extern ADC_HandleTypeDef hadc1;
extern volatile RunningState status;

void select_measure_adc() {
	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.Channel = ADC_CHANNEL_1;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	    Error_Handler();
}


void measure(Request request){
	uint8_t resolution = request.resolution;
	uint8_t measurementData[resolution];
	for(int i = 0; i < resolution; i++) measurementData[i] = 0x00;
	uint8_t intervalLength = (request.max_voltage - request.min_voltage)/resolution;
	uint8_t intervalSize = 4080 / resolution; // the maximum number of peaks that a category can store. 4080 = 255 * 16
	uint8_t peaks = 0;

	select_measure_adc();
	HAL_ADC_Start(&hadc1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 1);

	HAL_Delay(100);
	while(peaks < request.limit){
		uint8_t sample = sample_adc(request.samples, request.min_voltage, request.max_voltage, request.is_okay);
		if(!sample) break;
		uint8_t intervalIndex = abs(sample - request.min_voltage)/intervalLength;
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, peaks % 2);
		if(measurementData[intervalIndex] < intervalSize) measurementData[intervalIndex]++;
		if(request.type == MAX_HITS) peaks++;
	}

	HAL_ADC_Stop(&hadc1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 0);
	if(status != INTERRUPTED) {
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
	}else {
		add_error(request, INTERRUPT);
	}
	scheduler_finish_measurement();

}

/*
 * BEFORE YOU CALL make SURE ADC1 is INITIALIZED AND the selected channel is ADC_IN_0 !!!!
 */
uint8_t sample_adc(uint8_t samples, uint16_t min_voltage, uint16_t max_voltage, bool is_okay){
	uint32_t sum = 0;

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
	if(is_okay) {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		HAL_Delay(1);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	}
	return (uint8_t)(sum/samples);
}

int analogRead()
{
	HAL_ADC_PollForConversion(&hadc1, 100); // poll for conversion
	return HAL_ADC_GetValue(&hadc1); // get the adc value
}

