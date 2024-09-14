/*
 * Measurements.c
 *
 *  Created on: Aug 9, 2024
 *      Author: hadha
 */
#include"main.h"
#include"exp_i2c_slave.h"


const uint8_t triggerVoltage = 200;
#define TxSIZE 16
extern uint8_t TxData[TxSIZE];
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;
int ready = 1;
uint8_t voltage = 0;
int deltaTime = 0;


void measure()
{
	int readValue = 0;

	__HAL_TIM_SET_COUNTER(&htim1, 0);  // set the counter value a 0
	readValue = analogRead();
	voltage = (uint8_t)(readValue*255/4095);
	if(voltage > triggerVoltage){
		push((uint8_t)voltage);
		//deltaTime = __HAL_TIM_GET_COUNTER(&htim1);
		//push(deltaTime);

	}

}

int analogRead()
{
	int rv = 0;
	HAL_ADC_Start(&hadc1); // start the adc
	HAL_ADC_PollForConversion(&hadc1, 100); // poll for conversion
	rv = HAL_ADC_GetValue(&hadc1); // get the adc value
	return rv;
}

