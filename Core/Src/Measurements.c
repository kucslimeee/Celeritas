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

/**
 * TODO: Detailed documentation of the measurement process
 * Until it's completed please refer to Trello.
 */
void measure(Request request){

	uint8_t resolution = request.resolution;

	// `measurementData` must be at least 16 byte (or 8 uint16_t item) long
	// because we must produce a full packet (at least) per measurement.
	uint8_t arr_lenght = (resolution < 8 ) ? 8 : resolution;
	uint16_t measurementData[arr_length];

	uint16_t intervalLength = (request.max_voltage - request.min_voltage)/resolution; // the range of a single channel
	uint64_t peaks = 0;
	bool running = true;

	// ADC setup
	select_measurement_channel();
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED); 	// ADC auto calibration for single-ended input (has to be called before start)
	HAL_ADC_Start(&hadc1);									// Start the ADC

	// Start of the analog chain
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 1);

	// Measurement
	HAL_Delay(1000); // wait for the start of the analog chain
	while(running){
		// Get a sample
		uint16_t sample = sample_adc(request.samples, request.min_voltage, request.max_voltage, request.is_okay);
		if(!sample) break;

		// Calculating the channel and incrementing it
		uint8_t intervalIndex = abs(sample - request.min_voltage)/intervalLength; // have to discuss it
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, peaks % 2); // debug LED
		// if we are in "spectrum mode" (see trello card about it) and we haven't reached the limit of `uint16_t`,
		// then increment the channel
		if(resolution >= 8 && (measurementData[intervalIndex] + 1) < UINT16_MAX) {
			measurementData[intervalIndex]++;
		} else if(!request.continue_with_full_channel) running = false; // stop the request if we go just until the first filled channel

		// Run condition checking
		peaks++;
		if(request.type == MAX_HITS) {
			if(peaks == (uint64_t)request.limit) running = false;
		}
	}

	// Shutdown of the analog chain
	HAL_ADC_Stop(&hadc1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 0);

	// Saving the packet
	if(request.is_header){
		add_header(request, request.limit);
	}

	if(resolution < 8) {
		// "counting" mode: write peaks into `measurementData`
		measurementData[0] =  peaks >> 6;
		measurementData[1] = (peaks >> 4) & 0xFFFF;
		measurementData[2] = (peaks >> 2) & 0xFFFF;
		measurementData[3] =  peaks & 0xFFFF;
		add_spectrum(request, measurementData, resolution);
	} else {
		// "spectrum" mode
		uint8_t packets = arr_length / 8;
		for (uint8_t i = 1; i < packets; i++) {
			add_spectrum(request, measurementData+(i*8), resolution);
		}
	}

	// Tell the scheduler that we're done
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
		}while (voltage > min_voltage && status == RUNNING);
		if(okaying) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	}

	while(1){
		if(status != RUNNING) {
			return 0;
		}
		sum = analogRead(); //measure ADC
		if(sum > max_voltage) wait_for_min_threshold(true);
		if(!(sum > min_voltage && sum < max_voltage)) continue;
		for(int i = 1; i < samples; i++){
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
