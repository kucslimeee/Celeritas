/*
 * ReMeasure.c
 *
 * Created on: Sep 5, 2024
 * 	   Author: badam
 */

#include "Commands/ReMeasure.h"
#include "RequestQueue.h"
#include "Request.h"
#include "Scheduler.h"

extern void scheduler_add_request(uint8_t id, uint32_t start_time, uint8_t config);

void reMeasure(uint8_t id, uint8_t dec[]) {
    uint32_t start_time = (dec[3] << 24) | (dec[2] << 16) | (dec[1] << 8) | dec[0];
    scheduler_add_request(id, start_time, dec[4]);
}
