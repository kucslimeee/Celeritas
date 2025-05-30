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
bool flash_busy = false;
bool i2c_recieved = false;
bool backup_save = false;

volatile bool turnoff_check = false; // check if we need to care about current or next_request timesync
volatile RunningState status = IDLE;
RunningState status_before_sleep = IDLE;

extern Request empty_request;

// PRIVATE API

void scheduler_save_state();

bool validate_status() {
	if (status != IDLE && status != STARTING && status != RUNNING && status != FINISHED) {
		status = IDLE;
		return 0;
	}
	return 1;
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
	flash_load((uint32_t *)SCHEDULER_ADDR, 11, ((uint32_t *)&loaded_state)); // 1 read operation = 4 bytes (see flash_load docs)
	if(loaded_state[0] == 0xFFEE) backup_save = true;
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
	}
	// if the turnoff happened during the "countdown" of a request,
	// we need to check if it is still valid or not.
	if (status == STARTING) turnoff_check = true;
}

void scheduler_save_state() {
	uint16_t identifier = 0xFFFF;
	if (backup_save) identifier = 0xFFEE;
	uint16_t state[22] = {identifier, status};
	memcpy(state+2, &current_request, sizeof(Request));
	memcpy(state+12, &next_request, sizeof(Request));
	flash_save(SCHEDULER_ADDR, 1, 22, (uint16_t *)&state); // 1 write operation = 2 bytes of data (see flash_save docs)
}

void scheduler_clear_saved_state() {
	uint16_t identifier = 0xFFFF;
	uint16_t state[22];
	memset(state, 0, 22*sizeof(uint16_t));
	state[0] = identifier;
	state[1] = IDLE;
	flash_save(SCHEDULER_ADDR, 1, 22, (uint16_t *)&state); // 1 write operation = 2 bytes of data (see flash_save docs)
}

// PUBLIC METHOD IMPLEMENTATIONS

void scheduler_restart_sleeptimer() {
	sleep_timer = 300;
}

void scheduler_wakeup() {
	HAL_ResumeTick();
	if (status == SLEEP){
		status = status_before_sleep;
	}
}

void scheduler_on_command() {
	scheduler_on_even_second();
	command_complete = true;
}

void scheduler_restart() {
	restart_flag = true;
}

void scheduler_on_even_second() {
	if(current_request.type == MAX_TIME && status == RUNNING) {		//this condition stops the measurement process, when the requested time runs out
		duration--;
		if (duration == 0) status = FINISHED;
	}

	if (sleep_timer > 0) {		//sleep timer decrement
		sleep_timer--;
	}

	if(status != IDLE) return;		//if the status is not IDLE, then exit, otherwhys proceed
	uint32_t time = Get_SystemTime();

	if(next_request.ID > 0){					//check if the ID of a measurement is bigger than 0 (has to be)
		if(check_request(next_request, time)){	//check type and the time of the measurement
			current_request = next_request;		//start the measurement
			status = STARTING;
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
	i2c_recieved = true;
	if(status == RUNNING && interrupt_counter < 0xFF) {
		interrupt_counter++; //the measurement was interrupted
	}
}

void scheduler_on_timesync() {
	uint32_t system_time = Get_SystemTime();

	// if there was a turn off, we need to check current and next requests against
	// the first synchronized time
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
	if(i2c_recieved){
		i2c_recieved = false;
		scheduler_restart_sleeptimer();
		for (int i = 0; i < 5; i++){
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, i % 2 == 1);	//when i2c communication, double short LED flash
			HAL_Delay(50);
		}
	}

	if(command_complete) {
		command_complete = false;
		for (int i = 0; i < 5; i++){
					HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, i % 2 == 1);	//when command complete, double long LED flash
					HAL_Delay(200);
				}

		if (current_request.start_time - Get_SystemTime() < 600) {
			scheduler_restart_sleeptimer();
		}
	}

	validate_status();

	if (sleep_timer == 0) {
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
	if(Get_SystemTime() > current_request.start_time) {		//additional safety so the current request does not get stuck
		add_error(current_request.ID, TIMEOUT);
		current_request = empty_request;
		status = IDLE;
		return;
	}
	if(Get_SystemTime() != current_request.start_time) return;
	status = RUNNING;
	sleep_timer = 0;

	flash_busy = 1;			// flash_busy is responsible not to call flash_save twice by accident - this would lead to an error_handler or hardfault loop.
	scheduler_save_state(); // save the running state, to account for a sudden power outage with TERMINATED error after reboot
	flash_busy = 0;

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
	scheduler_restart_sleeptimer();
	flash_busy = 1;
	scheduler_save_state();
	flash_busy = 0;
}


void scheduler_add_request(uint8_t id, uint32_t start_time, uint8_t config) {
	bool instant_measurement = start_time == 0xffffffff; //if the start time is the maximum value,
	Request new_request;
	new_request.ID = id;
    new_request.type = getSetting(MODE_OF_OPERATION);
    new_request.is_okay = getSetting(IS_OKAY);
    new_request.is_header = config & 0x40; // the second bit of byte 5
    new_request.continue_with_full_channel = config & 0x80;
	new_request.limit = getSetting(DURATION);
	new_request.start_time = (instant_measurement) ? Get_SystemTime() + 2 : start_time; //then instantly schedule the measurement
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

void scheduler_request_selftest(uint8_t id, uint32_t start_time, uint8_t priority) {
	bool instant_measurement = start_time == 0xffffffff; //if the start time is the maximum value,
	Request new_request;
	new_request.ID =  id;
	new_request.type = SELFTEST;
	new_request.start_time = (instant_measurement) ? Get_SystemTime() + 2 : start_time; //then instantly schedule the measurement
	request_queue_put(new_request);
}

/*
 * 0 = current_request, 1 = next_request
 */
uint8_t scheduler_get_request_id(uint8_t idx) {
	uint8_t result = (idx) ? next_request.ID : current_request.ID;
	return result;
}

void scheduler_save_all() {
	if (flash_busy != 1){				//if the flash is busy saving something (there is a small chance that this happens),
		backup_save = true;				//then do not call the flash_save function again
		scheduler_save_state();
		saveSettings();
		queue_manager_save();
		i2c_queue_save();
		request_queue_save();
	}

}

void scheduler_clear_all_flash() {
	if (flash_busy != 1){				//if the flash is busy saving something (there is a small chance that this happens),
		backup_save = false;								//then do not call the flash_save function again
		scheduler_clear_saved_state();
		clear_saved_Settings();
		queue_manager_clear_saved();
		i2c_queue_clear_saved();
		request_queue_clear_saved();
	}
}
