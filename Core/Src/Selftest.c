/*
 * Selftest.c
 *
 *  Created on: Nov 14, 2024
 *      Author: hpraszpi
 */
#include "Selftest.h"
#include "i2c_queue.h"
#include "Timer.h"
#include "Measurements.h"
#include "Channels.h"
#include "main.h"
#include <stdint.h>
#include <stdbool.h>

extern ADC_HandleTypeDef hadc1;

void selftest(Request request) {
	bool error_filter (uint8_t* item) {
		return *(item+13) == 0xD5;
	}

	uint8_t error_count = queue_count(error_filter);
	uint16_t temperature = get_temperature();
	uint32_t time = Get_SystemTime();

	uint8_t fetch_packet_id(uint8_t idx) {
		bool success;
		uint8_t* packet = queue_fetch(idx, &success);
		if(success) {
			uint8_t type_byte = packet[14];
			if (type_byte == 0xD5 || type_byte == 0xFE) return packet[0];
			else return 0;
		}else return 0;
	}

	uint8_t packet_1 = fetch_packet_id(0);
	uint8_t packet_2 = fetch_packet_id(1);
	uint8_t request_1 = scheduler_get_request_id(0);
	uint8_t request_2 = scheduler_get_request_id(1);

	select_measurement_channel();
	HAL_ADC_Start(&hadc1);
	uint16_t test_measurement = analogRead();
	HAL_ADC_Stop(&hadc1);

	uint8_t packet_data[15] = {
		request.ID,
		(uint8_t)(temperature >> 8),
		(uint8_t)(temperature & 0xFF),
		error_count,
		(uint8_t)(time & 0xFF),
		(uint8_t)((time >> 8) & 0xFF),
		(uint8_t)((time >> 16) & 0xFF),
		(uint8_t)((time >> 24) & 0xFF),
		request_1,
		request_2,
		packet_1,
		packet_2,
		(uint8_t)(test_measurement & 0xFF),
		(uint8_t)((test_measurement >> 8) & 0xFF),
		0xFE
	};
	queue_push(&packet_data, request.is_priority, true);
	scheduler_finish_measurement();
}


