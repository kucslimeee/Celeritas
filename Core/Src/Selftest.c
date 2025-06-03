/*
 * Selftest.c
 *
 *  Created on: Nov 14, 2024
 *      Author: hpraszpi
 */
#include "Selftest.h"
#include "Scheduler.h"
#include "i2c_queue.h"
#include "Timer.h"
#include "Measurements.h"
#include "Channels.h"
#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#include "stm32f3xx_ll_adc.h"

extern ADC_HandleTypeDef hadc1;

void selftest(Request request) {
	bool error_filter (uint8_t* item) {
		return *(item+14) == 0xD5;
	}

	uint8_t error_count = i2c_queue_count(error_filter);
	uint16_t ref_voltage_temp = get_refint_voltage();
	int8_t temperature = get_temperature(ref_voltage_temp);
	uint32_t time = Get_SystemTime();

	uint8_t fetch_packet_id(uint8_t idx) {
		bool success;
		uint8_t* packet = i2c_queue_fetch(idx, &success);
		if(success) {
			uint8_t type_byte = packet[14];
			if (type_byte == 0xD5 || type_byte == 0xFE) return packet[0];
			else return 0;
		}else return 0;
	}

	uint8_t packet_1 = fetch_packet_id(0);
	uint8_t request_1 = scheduler_get_request_id(1);

	uint8_t boolean_byte = 0;
	if (backup_save) boolean_byte++;
	if (ABORTED) boolean_byte = boolean_byte + 2;



	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 1);
	HAL_Delay(250);										//Wait for the signal chain to start up and stabilize
	select_measurement_channel();
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED); 	// ADC auto calibration for single-ended input (has to be called before start)
	HAL_ADC_Start(&hadc1);									// Start the ADC

	uint8_t short_test_measurement = (uint8_t)analogRead();
	short_test_measurement = __LL_ADC_CALC_DATA_TO_VOLTAGE(ref_voltage_temp, short_test_measurement, LL_ADC_RESOLUTION_12B); //convert to mV

	uint32_t sum = 0;
	for (int b = 0; b < 1000; b++) {
		sum += analogRead();
		HAL_Delay(1);
	}
	uint16_t test_measurement = sum / 1000;
	HAL_ADC_Stop(&hadc1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 0);
	HAL_Delay(250);

	test_measurement = __LL_ADC_CALC_DATA_TO_VOLTAGE(ref_voltage_temp, test_measurement, LL_ADC_RESOLUTION_12B); //convert to mV

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);

	uint8_t packet_data[15] = {
		request.ID,
		temperature,
		short_test_measurement,
		error_count,
		(uint8_t)((time >> 24) & 0xFF),
		(uint8_t)((time >> 16) & 0xFF),
		(uint8_t)((time >> 8) & 0xFF),
		(uint8_t)(time & 0xFF),
		request_1,
		packet_1,
		boolean_byte,
		ref_voltage_temp >> 4,
		((ref_voltage_temp & 0xF) << 4) | (test_measurement >> 8),
		test_measurement & 0xFF,
		0xFE
	};
	i2c_queue_push(&packet_data, true, request.ID);
	scheduler_finish_measurement();
}


