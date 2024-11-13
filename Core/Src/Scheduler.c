/*
 * Scheduler.c
 *
 *  Created on: Nov 4, 2024
 *      Author: hpraszpi
 */

#include "Scheduler.h"
#include "Request.h"
#include "RequestQueue.h"
#include "i2c_queue.h"
#include "Timer.h"
#include "Measurements.h"
#include "SettingsStore.h"

// PRIVATE GLOBALS
volatile Request current_request;
volatile Request next_request;
uint16_t duration;
volatile RunningState status = IDLE;

// METHOD IMPLEMENTATIONS

void scheduler_on_even_second() {
	if(current_request.type == MAX_TIME) {
		duration--;
		if (duration == 0) status = FINISHED;
	}
	if(status != IDLE) return;
	if(next_request.start_time > 0) {
		current_request = next_request;
		status = STARTING;
	}

	uint32_t time = Get_SystemTime();
	for(int i = 0; i < 10; i++) {
		next_request = request_queue_get();
		if(next_request.start_time > 0) {
			if (time > next_request.start_time) {
				add_error(next_request, TIMEOUT);
				i = 0; // try 10 more times
			} else break;
		}
	}
}

void scheduler_on_i2c_communication() {
	if(status == RUNNING) status = INTERRUPTED;
}

void scheduler_update() {
	if(status != STARTING) return;
	if(Get_SystemTime() != current_request.start_time) return;
	status = RUNNING;
	measure(current_request);
}

void scheduler_finish_measurement() {
	status = IDLE;
}

void scheduler_add_request(uint8_t id, uint32_t start_time, uint8_t config) {
	Request new_request;
	new_request.ID = id;
    new_request.type = getSetting(MODE_OF_OPERATION);
    new_request.is_okay = getSetting(IS_OKAY);
    new_request.is_priority = config & 0x80; // the first bit of byte 5
    new_request.is_header = config & 0x40; // the second bit of byte 5
	new_request.limit = getSetting(DURATION);
	new_request.start_time = start_time;
	new_request.min_voltage = getSetting(MIN_VOLTAGE);
	new_request.max_voltage = getSetting(MAX_VOLTAGE);
	new_request.samples = getSetting(SAMPLES);
	new_request.resolution = getSetting(RESOLUTION);

	uint16_t repetitions = getSetting(REPETITIONS);
	if(repetitions == 0) {
		request_queue_put(new_request);
	} else {
		uint32_t current_start = new_request.start_time;
		uint16_t breaktime = getSetting(BREAKTIME);
		for(uint16_t i = 0; i < repetitions; i++) {
			Request adding_request = new_request;
			adding_request.start_time = current_start;
			request_queue_put(adding_request);
			current_start += (uint16_t)breaktime;
		}
	}
}


void scheduler_delete_request(uint8_t id) {
	request_queue_delete(id);
}

void scheduler_delete_all_requests() {
	request_queue_clear();
}
