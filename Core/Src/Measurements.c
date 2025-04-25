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
 * The measurement process of Celeritas.
 *
 * First it takes a Request as its configuration and sets up all the necessary data structures,
 * then starts the analog hardware chain, the actual measuring unit and enters the "measurement loop".
 *
 * The measurement loop first checks its running condition (in case of MAX_HITS measurement we do a fixed amount of
 * samples otherwise the limit is the maximum value of uint64_t) then gets a sample.
 * After we have our sample we can store it in two different ways, depending on the resolution of the measurement:
 *  - In "counting mode" (when the resolution is 1) we only count how many samples we've got and don't save any channels.
 *  - In "spectrum mode" (resolution >= 8) we fit the samples into channels (a given range of the whole measurement spectrum).
 *    With each sample we increment the appropriate channel and send how many samples we've had in that specific channel.
 * 	  Each channel is able to register 65535 samples and when one of them is reached this limit, we say it is filled.
 * 	  At this point we are able to interrupt the measurement process by setting the `request.continue_with_full_channel` option
 * 	  to false. Of course we can go forward, though we might lose samples in the spectrum of the filled channels since we can't
 * 	  register any new record.
 *
 * When we've finished with the measurement we shut down the analog measurement unit and store the results in i2c_queue.
 * At last we notify the Scheduler that we're finished.
 */
void measure(Request request){

	uint8_t resolution = request.resolution;

	// `measurementData` must be at least 16 byte (or 8 uint16_t item) long
	// because we must produce a full packet (at least) per measurement.
	uint8_t arr_length = (resolution < 8 ) ? 8 : resolution;
	uint16_t measurementData[arr_length];
	memset(measurementData, 0, arr_length*2);

	uint16_t intervalLength = (request.max_voltage - request.min_voltage)/resolution; // the range of a single channel
	uint64_t peaks = 0;

	// ADC setup
	select_measurement_channel();
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED); 	// ADC auto calibration for single-ended input (has to be called before start)
	HAL_ADC_Start(&hadc1);									// Start the ADC

	// Start of the analog chain
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 1);

	// Measurement
	HAL_Delay(1000); // wait for the start of the analog chain
	uint64_t peak_limit = (request.type == MAX_HITS) ? (uint64_t)request.limit : UINT64_MAX; // saved into memory to avoid this calculation at each loop
	for(peaks = 0; peaks < peak_limit; peaks++){
		// Get a sample
		uint16_t sample = sample_adc(request.samples, request.min_voltage, request.max_voltage, request.is_okay);
		if(!sample) break;
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, peaks % 2); // debug LED

		// Store the sample
		// note: at "counting mode" we don't have to execute all this stuff as we only count the number of peaks
		if (resolution >= 8) {
			// Calculating the channel and incrementing it
			uint8_t intervalIndex = abs(sample - request.min_voltage)/intervalLength; // have to discuss it
			if((measurementData[intervalIndex] + 1) < UINT16_MAX) {
				measurementData[intervalIndex]++;
			} else if(!request.continue_with_full_channel) {
				break; // stop the request if we go just until the first filled channel
			}
		}
	}

	// Shutdown of the analog chain
	HAL_Delay(1);	//just to make time to separately see the shutdown on the oscilloscope
	HAL_ADC_Stop(&hadc1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 0);
  
	// Saving the packet
	HAL_Delay(500);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);

	if(request.is_header){
		add_header(request, request.limit);
	}

	if(resolution < 8) {
		// "counting" mode: write peaks into `measurementData`
		measurementData[0] =  peaks >> 48;				// first 	two bytes 	0nd and 1st	(shift (8-2) * 8 = 48 bits)
		measurementData[1] = (peaks >> 32) & 0xFFFF;	// second 	two bytes	2nd and 3rd	(shift (8-4) * 8 = 32 bits)
		measurementData[2] = (peaks >> 16) & 0xFFFF;	// thrid 	two bytes	4th and 5th	(shift (8-6) * 8 = 16 bits)
		measurementData[3] =  peaks & 0xFFFF;			// fourth 	two bytes	6th and 7th	(shift (8-8) * 8 = 0 bits)
		add_spectrum(measurementData);
	} else {
		// "spectrum" mode

		// swapping the bytes inside of channels
		for (int i = 0; i < arr_length; i++)
			measurementData[i] = (measurementData[i] << 8) | (measurementData[i] >> 8);

		uint8_t packets = arr_length / 8;
		for (uint8_t i = 0; i < packets; i++) {
			add_spectrum(measurementData+(i*8));
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
