/*
 * Scheduler.h
 *
 *  Created on: Nov 4, 2024
 *      Author: hpraszpi
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_
#include <stdint.h>

typedef enum RunningState {
	STARTING,
	RUNNING,
	FINISHED,
	INTERRUPTED,
	IDLE
} RunningState;

extern volatile RunningState status;

void scheduler_on_even_second();
void scheduler_on_i2c_communication();
void scheduler_update();
void scheduler_finish_measurement();
void scheduler_add_request(uint8_t id, uint32_t start_time, uint8_t config);
void scheduler_delete_request(uint8_t id);
void scheduler_delete_all_requests();

#endif /* INC_SCHEDULER_H_ */
