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
#include "main.h"
#include "Measurements.h"
#include "SettingsStore.h"
#include <stdbool.h>
#include <string.h>
#include "Selftest.h"
#include "Flash.h"

// PRIVATE GLOBALS
volatile Request current_request;
volatile Request next_request;
uint16_t duration;
volatile uint8_t interrupt_counter;
volatile uint8_t sleep_timer;
bool command_complete = false;
bool restart_flag = false;
volatile RunningState status = IDLE;

// PRIVATE API
void scheduler_init() {
	uint16_t loaded_state[24];
	flash_load(SCHEDULER_ADDR, 24, &loaded_state);
	if(loaded_state[0] != 0xFFEE) return;
	Set_SystemTime(loaded_state[1] << 16 | loaded_state[2]);
	status = (RunningState)loaded_state[3];
	memcpy(&current_request, loaded_state+4, sizeof(Request));
	memcpy(&next_request, loaded_state+14, sizeof(Request));
}

void scheduler_save_state() {
	uint32_t time = Get_SystemTime();
	uint16_t state[24] = {0xFFEE, time >> 16, time & 0xFFFF, status};
	memcpy(state+4, &current_request, sizeof(Request));
	memcpy(state+14, &next_request, sizeof(Request));
	flash_save(SCHEDULER_ADDR, 24, &state);
}

// PUBLIC METHOD IMPLEMENTATIONS

void scheduler_enter_sleep() {
	sleep_timer = 60;
}

void scheduler_wakeup() {
	HAL_ResumeTick();
}

void scheduler_on_command() {
	scheduler_on_even_second();
	command_complete = true;
}

void scheduler_restart() {
	restart_flag = true;
	status = FINISHED;
}

void scheduler_on_even_second() {
	if(current_request.type == MAX_TIME && status == RUNNING) {
		duration--;
		if (duration == 0) status = FINISHED;
	}

	if (sleep_timer > 0) {
		if (sleep_timer-1 > 0) sleep_timer--;
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
				add_error(next_request.ID, TIMEOUT);
				i = 0; // try 10 more times
			} else break;
		}
	}
}

void scheduler_on_i2c_communication() {
	scheduler_wakeup();
	if(status != RUNNING) return;
	add_error(current_request.ID, INTERRUPT);
	if(interrupt_counter+1 <= 0xFF) interrupt_counter++;
}

void scheduler_update() {
	if(command_complete) {
		command_complete = false;
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
		HAL_Delay(200);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);
		if (!restart_flag) {
			i2c_queue_save();
			request_queue_save();
			scheduler_save_state();
		}
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
		HAL_Delay(200);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);

		if (Get_SystemTime()+120 >= current_request.start_time && current_request.type != UNKNOWN) {
			scheduler_wakeup();
		} else {
			scheduler_enter_sleep();
		}
	}
	if (status != IDLE && status != STARTING && status != RUNNING && status != FINISHED) {
		status = IDLE;
	}

	if (sleep_timer == 1) {
		sleep_timer = 0;
		i2c_queue_save();
		request_queue_save();
		scheduler_save_state();
		HAL_SuspendTick();
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 1);
		HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	}

	if(restart_flag) {
		restart_flag = false;
		HAL_NVIC_SystemReset();
	}

	if(status != STARTING) return;
	if(Get_SystemTime() != current_request.start_time) return;
	status = RUNNING;
	sleep_timer = 0;
	if (current_request.type == SELFTEST) selftest(current_request);
	else {
		if(current_request.type == MAX_TIME) duration = current_request.limit;
		measure(current_request);
	}
}

void scheduler_finish_measurement() {
	status = IDLE;
	interrupt_counter = 0;
	current_request = empty_request;
	i2c_queue_save();
	scheduler_enter_sleep();
}


void scheduler_add_request(uint8_t id, uint32_t start_time, uint8_t config) {
	bool instant_measurement = start_time == 0xffffffff;
	Request new_request;
	new_request.ID = id;
    new_request.type = getSetting(MODE_OF_OPERATION);
    new_request.is_okay = getSetting(IS_OKAY);
    new_request.is_header = config & 0x40; // the second bit of byte 5
    new_request.continue_with_full_channel = config & 0x80;
	new_request.limit = getSetting(DURATION);
	new_request.start_time = (instant_measurement) ? Get_SystemTime() + 2 : start_time;
	new_request.min_voltage = getSetting(MIN_VOLTAGE);
	new_request.max_voltage = getSetting(MAX_VOLTAGE);
	new_request.samples = getSetting(SAMPLES);
	new_request.resolution = getSetting(RESOLUTION);

	uint16_t repetitions = (instant_measurement) ? 0 : getSetting(REPETITIONS);
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
	request_queue_save();
}

void scheduler_request_selftest(uint8_t id, uint32_t start_time, uint8_t priority) {
	Request new_request;
	new_request.ID =  id;
	new_request.type = SELFTEST;
	bool instant_measurement = start_time == 0xffffffff;
	new_request.start_time = (instant_measurement) ? Get_SystemTime() + 2 : start_time;
	request_queue_put(new_request);
}

/*
 * 0 = current_request, 1 = next_request
 */
uint8_t scheduler_get_request_id(uint8_t idx) {
	return (idx) ? next_request.ID : current_request.ID;
}


void scheduler_delete_request(uint8_t id) {
	request_queue_delete(id);
}

void scheduler_delete_all_requests() {
	request_queue_clear();
}
