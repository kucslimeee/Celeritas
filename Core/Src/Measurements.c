/*
 * Measurements.c
 *
 *  Created on: Aug 9, 2024
 *      Author: hadha
 */
#include "Measurements.h"
#include "main.h"
#include "exp_i2c_slave.h"
#include <i2c_queue.h>

#define TxSIZE 16
#define RESOLUTION 15
#define SAMPLES 5

extern ADC_HandleTypeDef hadc1;

uint8_t sample_adc(uint8_t samples, uint16_t min_voltage, uint16_t max_voltage);

void max_hit_measurement(Request request){
	uint8_t measurementData[RESOLUTION] = {0x00};
	uint8_t intervalLength = (request.max_voltage - request.min_voltage)/RESOLUTION;
	uint8_t peaks = 0;
	HAL_ADC_Start(&hadc1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
	HAL_Delay(100);
	while(peaks < request.limit){
		uint8_t sample = sample_adc(SAMPLES, request.min_voltage, request.max_voltage);
		uint8_t intervalIndex = abs(sample - request.min_voltage)/intervalLength;
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, peaks % 2);
		measurementData[intervalIndex]++;
		peaks++;
	}


	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);
	if(request.is_header){
		if(request.is_priority){
			add_spectrum(request, &measurementData, RESOLUTION);
			add_header(request, request.limit);
		}
		else{
			add_header(request, request.limit);
			add_spectrum(request, &measurementData, RESOLUTION);
		}
	}
	else{
		//send data with a given priority
		add_spectrum(request, &measurementData, RESOLUTION);
	}

}

uint8_t sample_adc(uint8_t samples, uint16_t min_voltage, uint16_t max_voltage){
	uint32_t sum = 0;

	while(1){
		sum = analogRead(); //measure ADC
		if(!(sum > min_voltage && sum < max_voltage)) continue;
		for(int i = 1; i < samples; i++){
			sum += analogRead();
		}
		break;
	}
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	return (uint8_t)(sum/samples);
}

int analogRead()
{
	HAL_ADC_PollForConversion(&hadc1, 100); // poll for conversion
	return HAL_ADC_GetValue(&hadc1); // get the adc value
}

