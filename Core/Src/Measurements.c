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
#include <math.h>

#include "stm32f3xx_ll_adc.h"


extern ADC_HandleTypeDef hadc1;
extern volatile RunningState status;

uint16_t sample_adc(uint8_t samples, uint16_t min_voltage, uint16_t max_voltage, bool is_okay);

void measure(Request request){

	uint8_t resolution = request.resolution;
	uint8_t* measurementData[16] = {};
	uint16_t intervalLength = (request.max_voltage - request.min_voltage)/resolution;
	uint16_t peaks = 0;
	bool running = true;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 1);

	void incrementCategory(uint8_t interval) {
		#define INCREMENT(TYPE, SIZE)	\
		({						\
			if(*((TYPE*)measurementData+interval) < (SIZE)) { \
				*((TYPE*)measurementData+interval) += 1; \
			} \
			else if (!request.continue_with_full_channel) { \
				running = false; \
			}\
		})

		switch(resolution) {
		case 2:
			INCREMENT(uint64_t, UINT64_MAX);
			break;
		case 4:
			INCREMENT(uint32_t, UINT32_MAX);
			break;
		case 8:
			INCREMENT(uint16_t, UINT16_MAX);
			break;
		case 16:
			INCREMENT(uint8_t, UINT8_MAX);
			break;
		case 32:
		case 64:
		case 128:
			uint8_t intervalsPerByte = resolution / 16; // how many intervals are in a single byte
			uint8_t intervalSize = 8 / intervalsPerByte; // measured in bits
			uint8_t byteIndex = interval / intervalsPerByte;
			uint8_t intervalByte = *((uint8_t*)measurementData+byteIndex);
			uint8_t intervalLimit = 512 / resolution; // the max value of the interval
			uint8_t intervalIdx = interval - (byteIndex * intervalsPerByte); // the index of interval inside of a byte
			uint8_t intervalMask = (0xFF >> (8 - intervalSize));
			uint8_t intervalValue = (intervalByte >> ((intervalsPerByte - (intervalIdx + 1)) * intervalSize)) & intervalMask;
			if((intervalValue + 1) < intervalLimit)
				intervalByte += pow(2, (intervalSize * (intervalsPerByte - (intervalIdx + 1))));
			else if (!request.continue_with_full_channel)
				running = false;
			*((uint8_t*)measurementData+byteIndex) = intervalByte;
			break;
		default:
			break;
		}
	}

	HAL_Delay(500);
	select_measurement_channel();
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED); 	// ADC auto calibration for single-ended input (has to be called before start)
	HAL_ADC_Start(&hadc1);									// Start the ADC


	while(running){
		uint16_t sample = sample_adc(request.samples, request.min_voltage, request.max_voltage, request.is_okay);
		if(!sample) break;
		uint8_t intervalIndex = abs(sample - request.min_voltage)/intervalLength;
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, peaks % 2);
		incrementCategory(intervalIndex);
		peaks++;
		if(request.type == MAX_HITS) {
			if(peaks == request.limit) running = false;
		}
	}

	HAL_Delay(1);	//just to make time to separately see the shutdown on the oscilloscope
	HAL_ADC_Stop(&hadc1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 0);
	HAL_Delay(500);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);

	if(request.is_header){
		add_header(request, request.limit);
		add_spectrum(request, measurementData, resolution);
	}else {
		add_spectrum(request, measurementData, resolution);
	}
	scheduler_finish_measurement();
}

/*
 * BEFORE YOU CALL make SURE ADC1 is INITIALIZED AND the selected channel is ADC_IN_0 !!!!
 */
uint16_t sample_adc(uint8_t samples, uint16_t min_voltage, uint16_t max_voltage, bool is_okay){
	uint16_t sum = 0;

	void wait_for_min_threshold(bool okaying) {
		if(okaying) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		uint16_t voltage;
		do {
			voltage = analogRead();
		}while (voltage > (min_voltage - 20) && status == RUNNING);
		if(okaying) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	}

	while(1){
		if(status != RUNNING) {
			return 0;
		}
		sum = analogRead(); //measure ADC
		if(sum > max_voltage) wait_for_min_threshold(true);
		if(!(sum > (min_voltage + 20) && sum < (max_voltage - 20))) continue;
		for(int i = 1; i <= samples; i++){
			sum += analogRead();
		}
		break;
	}

	wait_for_min_threshold(is_okay);

	return (uint16_t)(sum/samples);
}

uint16_t analogRead()
{
	HAL_ADC_PollForConversion(&hadc1, 100); // poll for conversion
	return HAL_ADC_GetValue(&hadc1); // get the adc value
}


int get_temperature() {					//units: K (deg)
	select_temperature_channel();
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	int adc = HAL_ADC_GetValue(&hadc1);
	adc = __LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(4300, 1430, 298, 3300, adc, LL_ADC_RESOLUTION_12B);
	HAL_ADC_Stop(&hadc1);
	return adc;
}

int get_refint_voltage() {				//units: mV
	select_refint_channel();
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	int adc1 = HAL_ADC_GetValue(&hadc1);
	adc1 = __LL_ADC_CALC_VREFANALOG_VOLTAGE(adc1, LL_ADC_RESOLUTION_12B);
	HAL_ADC_Stop(&hadc1);
	return adc1;
}

//vbat channel is not connected?
/*int get_vbat_voltage() {
	select_vbat_channel();
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	int adc2 = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return adc2;
}*/
