/*
 * Measurements.c
 *
 *  Created on: Aug 9, 2024
 *      Author: hadha
 */
#include"main.h"
#include"exp_i2c_slave.h"
#include"Request.h"
#include<i2c_queue.h>
#include<Checksum.h>



#define TxSIZE 16

//extern uint8_t TxData[TxSIZE];
extern ADC_HandleTypeDef hadc1;
//extern TIM_HandleTypeDef htim1;



void max_hit_measurement(Request request, uint8_t resolution, uint8_t duration, uint8_t samples){
	uint8_t measurementData[resolution]; //max resolution 15
	uint8_t internalLength = (request.max_voltage - request.min_voltage)/resolution;
	uint8_t peaks = 0;
	uint8_t intervalIndex = 0;
	while(peaks < duration){
		intervalIndex = (uint8_t)((samples_adc(samples, request.min_voltage, request.max_voltage)
				-request.min_voltage)/internalLength);

		measurementData[intervalIndex]++;
		peaks++;
	}

	if(request.is_header){
		if(request.is_priority){
			//send data
			//send header
		}
		else{
			//send header
			//send data
		}
	}
	else{
		//send data with a given priority
	}

}

uint8_t sample_adc(uint8_t samples, uint8_t min_voltage, uint8_t max_voltage){
	uint8_t count = 0;
	uint8_t readVal;
	uint8_t lastSample;
	uint8_t SamplesData[samples];
	int sum = 0;

	while(count  < samples){
		readVal = (uint8_t)(analogRead()*255/4095); //measure ADC
		if(readVal > min_voltage && readVal < max_voltage){
			if(readVal > lastSample){
				SamplesData[count] = readVal;
				count++;
			}
			lastSample = readVal;
			sum += readVal;
		}

	}
	HAL_GPIO_WritePin(GPIOA, GPIO_Pin_6, GPIO_PIN_SET);
	HAL_GPIO_Delay(10);
	HAL_GPIO_WritePin(GPIOA, GPIO_Pin_6; GPIO_PIN_RESET);
	return (uint8_t)(sum/count);
}

int analogRead()
{
	int rv = 0;
	HAL_ADC_Start(&hadc1); // start the adc
	HAL_ADC_PollForConversion(&hadc1, 100); // poll for conversion
	rv = HAL_ADC_GetValue(&hadc1); // get the adc value
	return rv;
}

