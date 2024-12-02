/*
 * ReSelf.c
 *
 *  Created on: Nov 14, 2024
 *      Author: hpraszpi
 */

#include <Commands/ReSelftest.h>
#include "Scheduler.h"

void reSelftest(uint8_t command_id, uint8_t* dec) {
	 uint32_t start_time = (dec[3] << 24) | (dec[2] << 16) | (dec[1] << 8) | dec[0];
	 scheduler_request_selftest(command_id, start_time, dec[4]);
}
