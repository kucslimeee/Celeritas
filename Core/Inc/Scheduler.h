/*
 * Scheduler.h
 *
 *  Created on: Nov 4, 2024
 *      Author: hpraszpi
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_
#include <stdint.h>
#include "QueueManager.h"

typedef enum RunningState {
	SLEEP = 0x01,
	IDLE = 0x02,
	STARTING = 0x03,
	RUNNING = 0x04,
	FINISHED = 0x05,
} RunningState;

extern volatile RunningState status;
extern volatile uint8_t interrupt_counter;

void scheduler_init();
void scheduler_enter_sleep();
void scheduler_on_command();
void scheduler_on_timesync();
void scheduler_restart();
void scheduler_restart_sleeptimer();
void scheduler_on_even_second();
void scheduler_on_i2c_communication();
void scheduler_update();
void scheduler_finish_measurement();
void scheduler_request_measurement(uint8_t id, uint32_t start_time, uint8_t config);
void scheduler_request_selftest(uint8_t id, uint32_t start_time, uint8_t priority);

void scheduler_save_all();
void scheduler_clear_all_flash();
void scheduler_clear_saved_state();

void scheduler_add_request(uint8_t id, uint32_t start_time, uint8_t config);

uint8_t scheduler_get_request_id(uint8_t idx);

#endif /* INC_SCHEDULER_H_ */
