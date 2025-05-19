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
volatile uint16_t sleep_timer;
bool command_complete = false;
bool restart_flag = false;
volatile bool turnoff_check = false; // check if we need to care about current or next_request timesync
volatile RunningState status = IDLE;
RunningState status_before_sleep = IDLE;

// PRIVATE API

bool validate_status();
void scheduler_save_state();

bool validate_status() {
	if (status != IDLE && status != STARTING && status != RUNNING && status != FINISHED) {
		status = IDLE;
	}
}

/**	******************** SCHEDULER PREFERENCES ********************
 *  Scheduler preferences are system data that related to scheduler,
 *  but they aren't connected to any other subsystem for various re-
 *  asons.
 *
 *  Structure:
 *    - 0xFF 0xEE (mark of scheduler preferences -
 *    				labels that we have settings saved)
 *    - state (2 bytes);
 *    - current_request, next_request - 20-20 bytes each
 */

void scheduler_init() {
	uint16_t loaded_state[22];
	flash_load(SCHEDULER_ADDR, 11, ((uint32_t *)&loaded_state)); // 1 read operation = 4 bytes (see flash_load docs)
	if(loaded_state[0] != 0xFFEE) return;
	status = (RunningState)loaded_state[1];
	validate_status();
	memcpy(&current_request, loaded_state+2, sizeof(Request));
	memcpy(&next_request, loaded_state+12, sizeof(Request));

	// validate requests (if invalid - empty them)
	if (!check_request(current_request, 0)) status = IDLE;
	if (!check_request(next_request, 0)) next_request = empty_request;

	// check for turn offs while measurements
	// we need to detect if the system is stopped while running any measurement,
	// and generate an error if such thing had happened
	if (status == RUNNING) {
		add_error(current_request.ID, TERMINATED);
		current_request = empty_request;
		status = IDLE;
		turnoff_check = true;

		/*flash_access_halted = 1;
		scheduler_save_state(); // just that we don't generate two turnoff error for the same request
		flash_access_halted = 0;*/
	}
	// if the turnoff happened during the "countdown" of a request,
	// we need to check if it is still valid or not.
	if (status == STARTING) turnoff_check = true;
}

void scheduler_save_state() {
	uint16_t state[22] = {0xFFEE, status};
	memcpy(state+2, &current_request, sizeof(Request));
	memcpy(state+12, &next_request, sizeof(Request));
	flash_save(SCHEDULER_ADDR, 1, 22, (uint16_t *)&state); // 1 write operation = 2 bytes of data (see flash_save docs)
}

// PUBLIC METHOD IMPLEMENTATIONS

void scheduler_enter_sleep() {
	sleep_timer = 300;
}

void scheduler_wakeup() {
	HAL_ResumeTick();
	status = status_before_sleep;
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
	if(current_request.type == MAX_TIME && status == RUNNING) {		//this condition stops the measurement process, when the requested time runs out
		duration--;
		if (duration == 0) status = FINISHED;
	}

	if (sleep_timer > 1) {		//sleep timer decrement
		sleep_timer--;
	}

	if(status != IDLE) return;		//if the status is not IDLE, then exit, otherwhys proceed
	uint32_t time = Get_SystemTime();

	if(next_request.ID > 0){					//check if the ID of a measurement is bigger than 0 (has to be)
		if(check_request(next_request, time)){	//check type and the time of the measurement
			current_request = next_request;		//start the measurement
			status = STARTING;

			/*if(flash_access_halted != 1){
				flash_access_halted = 1;
				scheduler_save_state();
				flash_access_halted = 0;
			};*/						// save the starting sate

		} else current_request = empty_request;	//if the check_request returns false, then empty the request
	} else if (next_request.start_time > 0) {
		add_error(next_request.ID, TIMEOUT);	//if ID is 0, and the start_time is not 0, then give a TIMEOUT error
	}

	for(int i = 0; i < 10; i++) {						//try 10 times to find a request in the queue
		next_request = request_queue_get();
		if(next_request.start_time > 0) {
			if (time > next_request.start_time) {		// if a request should have been carried out in the past,
				add_error(next_request.ID, TIMEOUT);	// then give a timeout error and
				i = 0; 									// start over
			} else break;
		}
	}

	if (next_request.type != MAX_HITS && next_request.type != MAX_TIME && next_request.type != SELFTEST) {
		next_request = empty_request;		//empty the request
	}
}

void scheduler_on_i2c_communication() {
	scheduler_wakeup();
	if(status != RUNNING) return;
	if(interrupt_counter < 0xFF) interrupt_counter++;
}

void scheduler_on_timesync() {
	uint32_t system_time = Get_SystemTime();

	// if there was a turnoff, we need to check current and next requests agains
	// the first synced time
	if(turnoff_check) {
		if(!check_request(current_request, system_time)) {
			if(current_request.ID > 0) add_error(current_request.ID, TIMEOUT);
			status = IDLE;
		}
		if(!check_request(next_request, system_time)) {
			if(next_request.ID > 0) add_error(current_request.ID, TIMEOUT);
			next_request = empty_request;
		}
		turnoff_check = false;
	}
	scheduler_on_command();
}

void scheduler_update() {
	if(command_complete) {
		command_complete = false;
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);	//when command complete, double LED flash start
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);

		/*if (!restart_flag) {
			flash_access_halted = 1;
			scheduler_save_state();
			saveSettings();
			queue_manager_save();
			i2c_queue_save();
			request_queue_save();
			flash_access_halted = 0;
		}*/

		//double short led flash end
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
		HAL_Delay(100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);

		if (Get_SystemTime()+120 >= current_request.start_time && current_request.type != UNKNOWN) {
			scheduler_wakeup();
		} else {
			scheduler_enter_sleep();
		}
	}

	validate_status();

	if (sleep_timer == 1) {
		sleep_timer = 0;

		/*flash_access_halted = 1;
		scheduler_save_state();
		saveSettings();
		queue_manager_save();
		i2c_queue_save();
		request_queue_save();
		flash_access_halted = 0;*/

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 1);
		status_before_sleep = status;
		status = SLEEP;
		HAL_SuspendTick();
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

	/*flash_access_halted = 1;
	scheduler_save_state(); 	// save the running state
	flash_access_halted = 0;*/

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

	/*flash_access_halted = 1;
	i2c_queue_save();
	scheduler_save_state(); 		// save the idle state
	flash_access_halted = 0;*/

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
	/*flash_access_halted = 1;
	request_queue_save();
	flash_access_halted = 0;*/
}

void scheduler_request_selftest(uint8_t id, uint32_t start_time, uint8_t priority) {
	bool instant_measurement = start_time == 0xffffffff;
	Request new_request;
	new_request.ID =  id;
	new_request.type = SELFTEST;
	new_request.start_time = (instant_measurement) ? Get_SystemTime() + 2 : start_time;
	request_queue_put(new_request);

	/*flash_access_halted = 1;
	request_queue_save();
	flash_access_halted = 0;*/
}

/*
 * 0 = current_request, 1 = next_request
 */
uint8_t scheduler_get_request_id(uint8_t idx) {
	return (idx) ? next_request.ID : current_request.ID;
}
